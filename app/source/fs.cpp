#include <algorithm>
#include <cstring>
#include <filesystem>
#include <cstdarg>

#include "config.h"
#include "fs.h"
#include "kernel_functions.h"
#include "log.h"
#include "utils.h"

namespace FS {
    typedef struct {
        std::string  copy_path;
        std::string copy_filename;
        bool is_dir = false;
    } FSCopyEntry;
    
    static FSCopyEntry fs_copy_entry;
    
    bool FileExists(const std::string &path) {
        SceUID file = 0;
        
        if (R_SUCCEEDED(file = sceIoOpen(path.c_str(), PSP_O_RDONLY, 0777))) {
            sceIoClose(file);
            return true;
        }
        
        return false;
    }
    
    bool DirExists(const std::string &path) {	
        SceUID dir = 0;
        
        if (R_SUCCEEDED(dir = sceIoDopen(path.c_str()))) {
            sceIoDclose(dir);
            return true;
        }
        
        return false;
    }
    
    int MakeDir(const std::string &path) {
        int ret = 0;
        
        if (R_FAILED(ret = sceIoMkdir(path.c_str(), 0777)))
            return ret;
            
        return 0;
    }
    
    int CreateFile(const std::string &path) {
        SceUID file = 0;
        
        if (R_SUCCEEDED(file = sceIoOpen(path.c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777))) {
            sceIoClose(file);
            return 0;
        }
        
        return file;
    }
    
    std::string GetFileExt(const std::string &filename) {
        std::string ext = std::filesystem::path(filename).extension();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);
        return ext;
    }
    
    FileType GetFileType(const std::string &filename) {
        std::string ext = FS::GetFileExt(filename);

        if (!ext.compare(".PBP"))
            return FileTypeApp;
        else if ((!ext.compare(".ZIP")) || (!ext.compare(".RAR")) || (!ext.compare(".7Z")) || (!ext.compare(".LZMA")))
            return FileTypeArchive;
        else if ((!ext.compare(".FLAC")) || (!ext.compare(".IT")) || (!ext.compare(".MOD")) || (!ext.compare(".MP3")) || (!ext.compare(".OGG"))
            || (!ext.compare(".OPUS")) || (!ext.compare(".S3M")) || (!ext.compare(".WAV")) || (!ext.compare(".XM")))
            return FileTypeAudio;
        else if ((!ext.compare(".BMP")) || (!ext.compare(".GIF")) || (!ext.compare(".JPG")) || (!ext.compare(".JPEG")) || (!ext.compare(".PGM"))
            || (!ext.compare(".PPM")) || (!ext.compare(".PNG")) || (!ext.compare(".PSD")) || (!ext.compare(".TGA")) || (!ext.compare(".WEBP")))
            return FileTypeImage;
        else if ((!ext.compare(".JSON")) || (!ext.compare(".LOG")) || (!ext.compare(".TXT")) || (!ext.compare(".CFG")) || (!ext.compare(".INI")))
            return FileTypeText;
            
        return FileTypeNone;
    }
    
    SceOff GetFileSize(const std::string &path) {
        int ret = 0;
        
        SceIoStat stat;
        std::memset(&stat, 0, sizeof(stat));
        
        if (R_FAILED(ret = sceIoGetstat(path.c_str(), &stat)))
            return ret;
            
        return stat.st_size;
    }
    
    int ReadFile(const std::string &path, void *buf, int size) {
        SceUID file = 0;
        
        if (R_SUCCEEDED(file = sceIoOpen(path.c_str(), PSP_O_RDONLY, 0))) {
            int read = sceIoRead(file, buf, size);
            sceIoClose(file);
            return read;
        }
        
        return file;
    }
    
    int WriteFile(const std::string &path, void *buf, int size) {	
        SceUID file = 0;
        
        if (R_SUCCEEDED(file = sceIoOpen(path.c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777))) {
            int written = sceIoWrite(file, buf, size);
            sceIoClose(file);
            return written;
        }
        
        return file;
    }

    static bool Sort(const SceIoDirent &entryA, const SceIoDirent &entryB) {
        if ((FIO_S_ISDIR(entryA.d_stat.st_mode)) && !(FIO_S_ISDIR(entryB.d_stat.st_mode)))
            return true;
        else if (!(FIO_S_ISDIR(entryA.d_stat.st_mode)) && (FIO_S_ISDIR(entryB.d_stat.st_mode)))
            return false;
        else {
            switch(cfg.sort) {
                case 0:
                    if (strcasecmp(entryA.d_name, entryB.d_name) < 0)
                        return true;
                    break;

                case 1:
                    if (strcasecmp(entryB.d_name, entryA.d_name) < 0)
                        return true;
                    break;

                case 2:
                    if (entryB.d_stat.st_size < entryA.d_stat.st_size)
                        return true;
                    break;

                case 3:
                    if (entryA.d_stat.st_size < entryB.d_stat.st_size)
                        return true;
                    break;

                default:
                    break;
            }
        }
        
        return false;
    }

    int GetDirList(const std::string &path, std::vector<SceIoDirent> &entries) {
        int ret = 0;
        SceUID dir = 0;
        entries.clear();

        if (R_FAILED(ret = dir = sceIoDopen(path.c_str()))) {
            Log::Error("sceIoDopen(%s) failed: 0x%lx\n", path.c_str(), ret);
            return ret;
        }

        do {
            SceIoDirent entry;
            std::memset(&entry, 0, sizeof(entry));

            ret = sceIoDread(dir, &entry);
            if (ret > 0) {
                if ((std::strcmp(entry.d_name, ".") == 0) || (std::strcmp(entry.d_name, "..") == 0))
                    continue;

                entries.push_back(entry);
            }
        } while (ret > 0);

        std::sort(entries.begin(), entries.end(), FS::Sort);
        sceIoDclose(dir);
        return 0;
    }

    //TODO: Clean up change directory impl.
    static int ChangeDir(const std::string &path, std::vector<SceIoDirent> &entries) {
        int ret = 0;
        std::vector<SceIoDirent> new_entries;
        
        if (R_FAILED(ret = FS::GetDirList(path, new_entries)))
            return ret;
            
        // Free entries and change the current working directory.
        entries.clear();
        cfg.cwd = path;
        entries = new_entries;
        return 0;
    }

    static int ChangeDirUp(char path[256]) {
        if (cfg.cwd.back() == ':')
            return -1;
            
        // Remove upmost directory
        bool copy = false;
        int len = 0;
        for (ssize_t i = cfg.cwd.length(); i >= 0; i--) {
            if (cfg.cwd.c_str()[i] == '/')
                copy = true;
            if (copy) {
                path[i] = cfg.cwd.c_str()[i];
                len++;
            }
        }
        
        // remove trailing slash
        if (len > 1 && path[len - 1] == '/')
            len--;
        
        path[len] = '\0';
        return 0;
    }
    
    int ChangeDirNext(const std::string &path, std::vector<SceIoDirent> &entries) {
        std::string new_path = cfg.cwd;
        new_path.append("/");
        new_path.append(path);
        return FS::ChangeDir(new_path, entries);
    }
    
    int ChangeDirPrev(std::vector<SceIoDirent> &entries) {
        char new_path[256];
        if (FS::ChangeDirUp(new_path) < 0)
            return -1;
        
        return FS::ChangeDir(std::string(new_path), entries);
    }

    static int CopyFile(const std::string &src_path, const std::string &dest_path) {
        int ret = 0;
        SceUID src_handle = 0, dest_handle = 0;
        
        if (R_FAILED(ret = src_handle = sceIoOpen(src_path.c_str(), PSP_O_RDONLY, 0))) {
            Log::Error("sceIoOpen(%s) failed: 0x%x\n", src_path.c_str(), ret);
            return ret;
        }
        
        u64 size = sceIoLseek(src_handle, 0, PSP_SEEK_END);
        sceIoLseek(src_handle, 0, PSP_SEEK_SET);

        // Make sure we have enough storage to carry out this operation
        if (Utils::GetFreeStorage() < size) {
            Log::Error("Not enough storage is available to process this command 0x%x\n", src_path.c_str(), ret);
            sceIoClose(src_handle);
            return -1;
        }

        if (R_FAILED(ret = dest_handle = sceIoOpen(dest_path.c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777))) {
            Log::Error("sceIoOpen(%s) failed: 0x%x\n", dest_path.c_str(), ret);
            sceIoClose(src_handle);
            return ret;
        }
        
        u32 bytes_read = 0, bytes_written = 0;
        const u64 buf_size = 0x10000;
        u64 offset = 0;
        u8 *buf = new u8[buf_size];
        std::string filename = std::filesystem::path(src_path.data()).filename();
        
        do {
            if (Utils::IsCancelButtonPressed()) {
                delete[] buf;
                sceIoClose(src_handle);
                sceIoClose(dest_handle);
                return 0;
            }
            
            std::memset(buf, 0, buf_size);
            
            if (R_FAILED(ret = bytes_read = sceIoRead(src_handle, buf, buf_size))) {
                Log::Error("sceIoRead(%s) failed: 0x%x\n", src_path.c_str(), ret);
                delete[] buf;
                sceIoClose(src_handle);
                sceIoClose(dest_handle);
                return ret;
            }

            if (R_FAILED(ret = bytes_written = sceIoWrite(dest_handle, buf, bytes_read))) {
                Log::Error("sceIoWrite(%s) failed: 0x%x\n", dest_path.c_str(), ret);
                delete[] buf;
                sceIoClose(src_handle);
                sceIoClose(dest_handle);
                return ret;
            }
            
            offset += bytes_read;
            //GUI::ProgressBar("Copying", filename.c_str(), offset, size);
        } while(offset < size);
        
        delete[] buf;
        sceIoClose(src_handle);
        sceIoClose(dest_handle);
        return 0;
    }
    
    static int CopyDir(const std::string &src_path, const std::string &dest_path) {
        int ret = 0;
        SceUID dir;

        if (R_FAILED(dir = sceIoDopen(src_path.c_str()))) {
            Log::Error("sceIoDopen(%s) failed: 0x%lx\n", src_path.c_str(), ret);
            return dir;
        }
        
        // This may fail or not, but we don't care -> make the dir if it doesn't exist, otherwise continue.
        sceIoMkdir(dest_path.c_str(), 0777);
        
        do {
            SceIoDirent entry;
            std::memset(&entry, 0, sizeof(entry));

            ret = sceIoDread(dir, &entry);
            if (ret > 0) {
                if ((std::strcmp(entry.d_name, ".") == 0) || (std::strcmp(entry.d_name, "..") == 0))
                    continue;
                    
                std::string src = src_path;
                src.append("/");
                src.append(entry.d_name);
                
                std::string dest = dest_path;
                dest.append("/");
                dest.append(entry.d_name);
                
                if (FIO_S_ISDIR(entry.d_stat.st_mode))
                    FS::CopyDir(src, dest); // Copy Folder (via recursion)
                else
                    FS::CopyFile(src, dest); // Copy File
            }
        } while (ret > 0);
        
        sceIoDclose(dir);
        return 0;
    }

    static void ClearCopyData(void) {
        fs_copy_entry.copy_path.clear();
        fs_copy_entry.copy_filename.clear();
        fs_copy_entry.is_dir = false;
    }
    
    void Copy(SceIoDirent *entry, const std::string &path) {
        FS::ClearCopyData();
        fs_copy_entry.copy_path = path;
        fs_copy_entry.copy_path.append("/");
        fs_copy_entry.copy_path.append(entry->d_name);
        fs_copy_entry.copy_filename.append(entry->d_name);
        
        if (FIO_S_ISDIR(entry->d_stat.st_mode))
            fs_copy_entry.is_dir = true;
    }
    
    int Paste(void) {
        int ret = 0;
        std::string path = cfg.cwd;
        path.append("/");
        path.append(fs_copy_entry.copy_filename);
        
        if (fs_copy_entry.is_dir) // Copy folder recursively
            ret = FS::CopyDir(fs_copy_entry.copy_path, path);
        else // Copy file
            ret = FS::CopyFile(fs_copy_entry.copy_path, path);
            
        FS::ClearCopyData();
        return ret;
    }
    
    // Thanks to TN for finding this.
    static int sceIoMove(const char *src, const char *dest) {
        int ret = 0;
        size_t i = 0;
        char strage[32];
        char *p1 = nullptr, *p2 = nullptr;
        p1 = std::strchr(src, ':');
        
        if (p1 == nullptr)
            return -1;
            
        p2 = std::strchr(dest, ':');
        if (p2 == nullptr)
            return -1;
            
        if ((p1 - src) != (p2 - dest))
            return -1;
            
        for (i = 0; (src + i) <= p1; i++) {
            if ((i+1) >= sizeof(strage))
                return -1;
                
            if (src[i] != dest[i])
                return -1;
            
            strage[i] = src[i];
        }
        
        strage[i] = '\0';

        u32 data[2];
        data[0] = (u32)(p1 + 1);
        data[1] = (u32)(p2 + 1);
        
        if (R_FAILED(ret = sceIoDevctl(strage, 0x02415830, &data, sizeof(data), nullptr, 0))) {
            Log::Error("sceIoDevctl() failed!", ret);
            return ret;
        }
        
        return 0;
    }

    int Move(void) {
        int ret = 0;
        std::string path = cfg.cwd;
        path.append("/");
        path.append(fs_copy_entry.copy_filename);

        if (R_FAILED(ret = sceIoMove(fs_copy_entry.copy_path.c_str(), path.c_str()))) {
            Log::Error("sceIoMove(%s, %s) failed: 0x%x\n", fs_copy_entry.copy_filename.c_str(), path.c_str(), ret);
            FS::ClearCopyData();
            return ret;
        }
        
        FS::ClearCopyData();
        return 0;
    }

    static int DeleteDirectoryRecursive(const std::string &path) {
        int ret = 0;
        SceUID dir = 0;

        if (R_FAILED(ret = dir = sceIoDopen(path.c_str()))) {
            if (R_FAILED(ret = sceIoRemove(path.c_str()))) {
                Log::Error("sceIoRemove(%s) failed: 0x%lx\n", path.c_str(), ret);
                return ret;
            }
        }

        do {
            SceIoDirent entry;
            std::memset(&entry, 0, sizeof(entry));

            ret = sceIoDread(dir, &entry);
            if (ret > 0) {
                if ((std::strcmp(entry.d_name, ".") == 0) || (std::strcmp(entry.d_name, "..") == 0))
                    continue;

                std::string new_path = path;
                new_path.append("/");
                new_path.append(entry.d_name);
                
                if (FIO_S_ISDIR(entry.d_stat.st_mode)) {
                    int result = FS::DeleteDirectoryRecursive(new_path);
                    if (result <= 0) {
                        Log::Error("FS::DeleteDirectoryRecursive(%s) failed: 0x%lx\n", path.c_str(), ret);
                        sceIoDclose(dir);
                        return ret;
                    }
                }
                else {
                    int result = sceIoRemove(new_path.c_str());
                    if (R_FAILED(result)) {
                        Log::Error("sceIoRemove(%s) failed: 0x%lx\n", path.c_str(), ret);
                        sceIoDclose(dir);
                        return ret;
                    }
                }
            }
        } while (ret > 0);
        
        sceIoDclose(dir);

        if (R_FAILED(ret = sceIoRmdir(path.c_str()))) {
            Log::Error("sceIoRmdir(%s) failed: 0x%lx\n", path.c_str(), ret);
            return ret;
        }

        return 1;
    }

    int Delete(SceIoDirent *entry) {
        int ret = 0;
        std::string path = cfg.cwd;
        path.append("/");
        path.append(entry->d_name);

        if (FIO_S_ISDIR(entry->d_stat.st_mode)) {
            if (R_FAILED(ret = FS::DeleteDirectoryRecursive(path))) {
                Log::Error("FS::DeleteDirectoryRecursive(%s) failed: 0x%x\n", path.c_str(), ret);
                return ret;
            }
        }
        else {
            if (R_FAILED(ret = sceIoRemove(path.c_str()))) {
                Log::Error("sceIoRemove(%s) failed: 0x%x\n", path.c_str(), ret);
                return ret;
            }
        }

        return 0;
    }
}