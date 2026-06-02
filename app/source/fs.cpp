#include <algorithm>
#include <cstring>
#include <filesystem>
#include <psppower.h>

#include "config.h"
#include "fs.h"
#include "gui.h"
#include "kernel_functions.h"
#include "log.h"
#include "utils.h"

namespace FS {
    typedef struct {
        std::string  path;
        std::string filename;
        bool isDir = false;
    } FSCopyEntry;
    
    static FSCopyEntry fs_copy_entry;
    
    bool FileExists(const char *path) {
        SceIoStat stat;
#ifdef FS_DEBUG
        return sceIoGetstat(path, &stat) >= 0;
#else
        return pspIoGetstat(path, &stat) >= 0;
#endif
    }
    
    bool DirExists(const char *path) {	
        SceUID dir = 0;
#ifdef FS_DEBUG
        if (R_SUCCEEDED(dir = sceIoDopen(path))) {
            sceIoDclose(dir);
#else
        if (R_SUCCEEDED(dir = pspIoOpenDir(path))) {
            pspIoCloseDir(dir);
#endif
            return true;
        }
        
        return false;
    }
    
    int MakeDir(const std::string &path) {
        int ret = 0;
#ifdef FS_DEBUG
        if (R_FAILED(ret = sceIoMkdir(path.c_str(), 0777))) {
#else
        if (R_FAILED(ret = pspIoMakeDir(path.c_str(), 0777))) {
#endif
            return ret;
        }
            
        return 0;
    }

    // https://newbedev.com/mkdir-c-function
    int RecursiveMakeDir(const std::string &path) {
        std::string currentLevel = "";
        std::string level;
        std::stringstream ss(path);
        
        // split path using slash as a separator
        while (std::getline(ss, level, '/')) {
            currentLevel += level; // append folder to the current level
            
            // create current level
            if (!FS::DirExists(currentLevel.c_str()) && FS::MakeDir(currentLevel.c_str()) != 0) {
                return -1;
            }
                
            currentLevel += "/"; // don't forget to append a slash
        }
        
        return 0;
    }
    
    int CreateFile(const char *path) {
        SceUID file = 0;

#ifdef FS_DEBUG
        if (R_SUCCEEDED(file = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777))) {
            sceIoClose(file);
            return 0;
        }
#else
        if (R_SUCCEEDED(file = pspIoOpenFile(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777))) {
            pspIoCloseFile(file);
            return 0;
        }
#endif
        
        return file;
    }
    
    const char* GetFileExt(const char *filename) {
        const char *ext = strrchr(filename, '.');
        
        if (ext == nullptr) {
            return "";
        }
        
        return ext + 1;
    }

    FileType GetFileType(const char *filename) {
        const char *ext = FS::GetFileExt(filename);

        if ((strncasecmp(ext, "cso", 3) == 0) || (strncasecmp(ext, "iso", 3) == 0) || (strncasecmp(ext, "pbp", 3) == 0)) {
            return FileTypeApp;
        }
        else if ((strncasecmp(ext, "7z", 2) == 0) || (strncasecmp(ext, "lzma", 4) == 0) || (strncasecmp(ext, "rar", 3) == 0)
            || (strncasecmp(ext, "tar", 3) == 0) || (strncasecmp(ext, "zip", 3) == 0)) {
            return FileTypeArchive;
        }
        else if ((strncasecmp(ext, "flac", 4) == 0) || (strncasecmp(ext, "it", 2) == 0) || (strncasecmp(ext, "mod", 3) == 0)
            || (strncasecmp(ext, "mp3", 3) == 0) || (strncasecmp(ext, "ogg", 3) == 0) || (strncasecmp(ext, "opus", 4) == 0)
            || (strncasecmp(ext, "s3m", 3) == 0) || (strncasecmp(ext, "wav", 3) == 0) || (strncasecmp(ext, "xm", 2) == 0)) {
                return FileTypeAudio;
        }
        else if ((strncasecmp(ext, "bmp", 3) == 0) || (strncasecmp(ext, "gif", 3) == 0) || (strncasecmp(ext, "jpg", 3) == 0)
            || (strncasecmp(ext, "jpeg", 4) == 0) || (strncasecmp(ext, "pgm", 3) == 0) || (strncasecmp(ext, "ppm", 3) == 0)
            || (strncasecmp(ext, "png", 3) == 0) || (strncasecmp(ext, "psd", 3) == 0) || (strncasecmp(ext, "tga", 3) == 0)
            || (strncasecmp(ext, "tif", 3) == 0) || (strncasecmp(ext, "tiff", 4) == 0)) {
                return FileTypeImage;
        }
        else if ((strncasecmp(ext, "cfg", 3) == 0) || (strncasecmp(ext, "conf", 4) == 0) || (strncasecmp(ext, "ini", 3) == 0)
            || (strncasecmp(ext, "json", 4) == 0) || (strncasecmp(ext, "log", 3) == 0) || (strncasecmp(ext, "md", 2) == 0)
            || (strncasecmp(ext, "txt", 3) == 0)) {
            return FileTypeText;
        }
            
        return FileTypeNone;
    }
    
    SceOff GetFileSize(const char *path) {
        int ret = 0;
        
        SceIoStat stat;
        std::memset(&stat, 0, sizeof(stat));
        
#ifdef FS_DEBUG
        if (R_FAILED(ret = sceIoGetstat(path, &stat))) {
#else
        if (R_FAILED(ret = pspIoGetstat(path, &stat))) {
#endif
            return ret;
        }
            
        return stat.st_size;
    }
    
    char *GetFileTimestamp(SceIoStat &stat, FileTimestamp time) {
        static char timestamp[30];

        switch(time) {
            case FileCreatedTime:
                std::snprintf(timestamp, 30, "%d/%d/%d %2i:%02i", stat.sce_st_ctime.year, stat.sce_st_ctime.month, stat.sce_st_ctime.day, stat.sce_st_ctime.hour,
                    stat.sce_st_ctime.minute);
                break;
                
            case FileAccessedTime:
                std::snprintf(timestamp, 30, "%d/%d/%d %2i:%02i", stat.sce_st_atime.year, stat.sce_st_atime.month, stat.sce_st_atime.day, stat.sce_st_atime.hour,
                    stat.sce_st_atime.minute);
                break;
            
            case FileModifiedTime:
                std::snprintf(timestamp, 30, "%d/%d/%d %2i:%02i", stat.sce_st_mtime.year, stat.sce_st_mtime.month, stat.sce_st_mtime.day, stat.sce_st_mtime.hour,
                    stat.sce_st_mtime.minute);
                break;
        }
        
        return timestamp;
    }

    char *GetFilePermission(SceIoStat &stat) {
        static char perms[11];

        std::snprintf(perms, 11, "%s%s%s%s%s%s%s%s%s%s", (FIO_S_ISDIR(stat.st_mode)) ? "d" : "-", (stat.st_mode & FIO_S_IRUSR) ? "r" : "-",
            (stat.st_mode & FIO_S_IWUSR) ? "w" : "-", (stat.st_mode & FIO_S_IXUSR) ? "x" : "-", (stat.st_mode & FIO_S_IRGRP) ? "r" : "-",
            (stat.st_mode & FIO_S_IWGRP) ? "w" : "-", (stat.st_mode & FIO_S_IXGRP) ? "x" : "-", (stat.st_mode & FIO_S_IROTH) ? "r" : "-",
            (stat.st_mode & FIO_S_IWOTH) ? "w" : "-", (stat.st_mode & FIO_S_IXOTH) ? "x" : "-");
            
        return perms;
    }
    
    int ReadFile(const std::string &path, void *buf, int size) {
        SceUID file = 0;

#ifdef FS_DEBUG
        if (R_SUCCEEDED(file = sceIoOpen(path.c_str(), PSP_O_RDONLY, 0))) {
            int read = sceIoRead(file, buf, size);
            sceIoClose(file);
            return read;
        }
#else
        if (R_SUCCEEDED(file = pspIoOpenFile(path.c_str(), PSP_O_RDONLY, 0))) {
            int read = pspIoReadFile(file, buf, size);
            pspIoCloseFile(file);
            return read;
        }
#endif
        
        return file;
    }
    
    int WriteFile(const std::string &path, void *buf, int size) {	
        SceUID file = 0;
        
#ifdef FS_DEBUG
        if (R_SUCCEEDED(file = sceIoOpen(path.c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777))) {
            int written = sceIoWrite(file, buf, size);
            sceIoClose(file);
            return written;
        }
#else
        if (R_SUCCEEDED(file = pspIoOpenFile(path.c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777))) {
            int written = pspIoWriteFile(file, buf, size);
            pspIoCloseFile(file);
            return written;
        }
#endif
        
        return file;
    }

    static bool Sort(const SceIoDirent &a, const SceIoDirent &b) {
        bool aIsDir = FIO_S_ISDIR(a.d_stat.st_mode);
        bool bIsDir = FIO_S_ISDIR(b.d_stat.st_mode);
    
        // Directories first
        if (aIsDir != bIsDir) {
            return aIsDir;
        }
    
        switch (cfg.sort) {
            case 0: // Name A-Z
                return strcasecmp(a.d_name, b.d_name) < 0;
    
            case 1: // Name Z-A
                return strcasecmp(a.d_name, b.d_name) > 0;
    
            case 2: // Size descending
                return a.d_stat.st_size > b.d_stat.st_size;
    
            case 3: // Size ascending
                return a.d_stat.st_size < b.d_stat.st_size;
    
            default:
                return false;
        }
    }
    
    int GetDirList(const std::string &path, std::vector<SceIoDirent> &entries) {
        int ret = 0;
        SceUID dir = 0;
        entries.clear();

#ifdef FS_DEBUG
        if (R_FAILED(ret = dir = sceIoDopen(path.c_str()))) {
            Log::Error("%s(sceIoDopen) failed: %s (0x%08x)\n", __func__, path.c_str(), ret);
            return ret;
        }
#else
        if (R_FAILED(ret = dir = pspIoOpenDir(path.c_str()))) {
            Log::Error("%s(pspIoOpenDir) failed: %s (0x%08x)\n", __func__, path.c_str(), ret);
            return ret;
        }
#endif
        dir = ret;
        SceIoDirent entry;
    
        while (true) {
            std::memset(&entry, 0, sizeof(entry));
#ifdef FS_DEBUG
            ret = sceIoDread(dir, &entry);
#else
            ret = pspIoReadDir(dir, &entry);
#endif
            if (ret <= 0) {
                break;
            }
    
            if (entry.d_name[0] == '.' && (entry.d_name[1] == '\0' || (entry.d_name[1] == '.' && entry.d_name[2] == '\0'))) {
                continue; // skip "." and ".."
            }
    
            entries.push_back(entry);
        }
    
#ifdef FS_DEBUG
        sceIoDclose(dir);
#else
        pspIoCloseDir(dir);
#endif
    
        std::sort(entries.begin(), entries.end(), FS::Sort);
        return 0;
    }

    static int ChangeDir(const std::string &path, std::vector<SceIoDirent> &entries) {
        int ret = 0;
        std::vector<SceIoDirent> newEntries;
        
        if (R_FAILED(ret = FS::GetDirList(path, newEntries))) {
            return ret;
        }
        
        entries.swap(newEntries);
        cfg.cwd = path;
        return 0;
    }

    int ChangeDirNext(const std::string &path, std::vector<SceIoDirent> &entries) {
        const std::string newPath = FS::BuildPath(cfg.cwd, path);
        return FS::ChangeDir(newPath, entries);
    }
    
    int ChangeDirPrev(std::vector<SceIoDirent> &entries) {
        std::string parentPath = std::filesystem::path(cfg.cwd).parent_path().string();
        
        if (!parentPath.empty() && parentPath.back() == ':') {
            parentPath += '/';
        }
        
        return FS::ChangeDir(parentPath.empty()? cfg.cwd : parentPath, entries);
    }
    
    std::string GetFilename(const std::string &path) {
        std::size_t pos = path.find_last_of("/\\");
        return (pos == std::string::npos)? path : path.substr(pos + 1);
    }

    static int CopyFile(const std::string &src_path, const std::string &destPath) {
        int ret = 0;
        SceUID srcHandle = 0, destHandle = 0;
        scePowerLock(0);

#ifdef FS_DEBUG
        if (R_FAILED(ret = srcHandle = sceIoOpen(src_path.c_str(), PSP_O_RDONLY, 0))) {
            Log::Error("sceIoOpen(%s) failed: 0x%x\n", src_path.c_str(), ret);
            scePowerUnlock(0);
            return ret;
        }
#else
        if (R_FAILED(ret = srcHandle = pspIoOpenFile(src_path.c_str(), PSP_O_RDONLY, 0))) {
            Log::Error("pspIoOpenFile(%s) failed: 0x%x\n", src_path.c_str(), ret);
            scePowerUnlock(0);
            return ret;
        }
#endif

#ifdef FS_DEBUG
        u64 size = sceIoLseek(srcHandle, 0, PSP_SEEK_END);
        sceIoLseek(srcHandle, 0, PSP_SEEK_SET);
#else
        u64 size = pspIoLseek(srcHandle, 0, PSP_SEEK_END);
        pspIoLseek(srcHandle, 0, PSP_SEEK_SET);
#endif

        // Make sure we have enough storage to carry out this operation
        if (Utils::GetFreeStorage() < size) {
            Log::Error("Not enough storage is available to process this command 0x%x\n", src_path.c_str(), ret);
#ifdef FS_DEBUG
            sceIoClose(srcHandle);
#else
            pspIoCloseFile(srcHandle);
#endif
            scePowerUnlock(0);
            return -1;
        }

        if (R_FAILED(ret = destHandle = sceIoOpen(destPath.c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777))) {
            Log::Error("sceIoOpen(%s) failed: 0x%x\n", destPath.c_str(), ret);
#ifdef FS_DEBUG
            sceIoClose(srcHandle);
#else
            pspIoCloseFile(srcHandle);
#endif
            scePowerUnlock(0);
            return ret;
        }
        
        u32 bytesRead = 0, bytesWritten = 0;
        const u64 bufSize = 0x10000;
        u64 offset = 0;
        u8 *buf = new u8[bufSize];
        std::string filename = std::filesystem::path(src_path.data()).filename();
        
        do {
            if (Utils::IsCancelButtonPressed()) {
                delete[] buf;
#ifdef FS_DEBUG
                sceIoClose(srcHandle);
                sceIoClose(destHandle);
#else
                pspIoCloseFile(srcHandle);
                pspIoCloseFile(destHandle);
#endif
                scePowerUnlock(0);
                return 0;
            }
            
            std::memset(buf, 0, bufSize);

#ifdef FS_DEBUG        
            if (R_FAILED(ret = bytesRead = sceIoRead(srcHandle, buf, bufSize))) {
                Log::Error("sceIoRead(%s) failed: 0x%x\n", src_path.c_str(), ret);
#else
            if (R_FAILED(ret = bytesRead = pspIoReadFile(srcHandle, buf, bufSize))) {
                Log::Error("pspIoReadFile(%s) failed: 0x%x\n", src_path.c_str(), ret);
#endif
                delete[] buf;
#ifdef FS_DEBUG
                sceIoClose(srcHandle);
                sceIoClose(destHandle);
#else
                pspIoCloseFile(srcHandle);
                pspIoCloseFile(destHandle);
#endif
                scePowerUnlock(0);
                return ret;
            }

#ifdef FS_DEBUG
            if (R_FAILED(ret = bytesWritten = sceIoWrite(destHandle, buf, bytesRead))) {
                Log::Error("sceIoWrite(%s) failed: 0x%x\n", destPath.c_str(), ret);
#else
            if (R_FAILED(ret = bytesWritten = pspIoWriteFile(destHandle, buf, bytesRead))) {
                Log::Error("pspIoWriteFile(%s) failed: 0x%x\n", destPath.c_str(), ret);
#endif
                delete[] buf;
#ifdef FS_DEBUG
                sceIoClose(srcHandle);
                sceIoClose(destHandle);
#else
                pspIoCloseFile(srcHandle);
                pspIoCloseFile(destHandle);
#endif
                scePowerUnlock(0);
                return ret;
            }
            
            offset += bytesRead;
            GUI::ProgressBar("Copying", filename.c_str(), offset, size);
        } while(offset < size);
        
        delete[] buf;
#ifdef FS_DEBUG
        sceIoClose(srcHandle);
        sceIoClose(destHandle);
#else
        pspIoCloseFile(srcHandle);
        pspIoCloseFile(destHandle);
#endif
        scePowerUnlock(0);
        return 0;
    }
    
    static int CopyDir(const std::string &src_path, const std::string &destPath) {
        int ret = 0;
        SceUID dir;

#ifdef FS_DEBUG
        if (R_FAILED(ret = dir = sceIoDopen(src_path.c_str()))) {
            Log::Error("sceIoDopen(%s) failed: 0x%08x\n", src_path.c_str(), ret);
            return ret;
        }
#else
        if (R_FAILED(ret = dir = pspIoOpenDir(src_path.c_str()))) {
            Log::Error("pspIoOpenDir(%s) failed: 0x%08x\n", src_path.c_str(), ret);
            return dir;
        }
#endif
        
        // This may fail or not, but we don't care -> make the dir if it doesn't exist, otherwise continue.
#ifdef FS_DEBUG
        sceIoMkdir(destPath.c_str(), 0777);
#else
        pspIoMakeDir(destPath.c_str(), 0777);
#endif
        
        do {
            SceIoDirent entry;
            std::memset(&entry, 0, sizeof(entry));

#ifdef FS_DEBUG
            ret = sceIoDread(dir, &entry);
#else
            ret = pspIoReadDir(dir, &entry);
#endif
            if (ret > 0) {
                if ((std::strcmp(entry.d_name, ".") == 0) || (std::strcmp(entry.d_name, "..") == 0))
                    continue;
                    
                std::string src = FS::BuildPath(src_path, entry.d_name);
                std::string dest = FS::BuildPath(destPath, entry.d_name);
                
                if (FIO_S_ISDIR(entry.d_stat.st_mode))
                    FS::CopyDir(src, dest); // Copy Folder (via recursion)
                else
                    FS::CopyFile(src, dest); // Copy File
            }
        } while (ret > 0);
        
#ifdef FS_DEBUG
        sceIoDclose(dir);
#else
        pspIoCloseDir(dir);
#endif
        return 0;
    }

    static void ClearCopyData(void) {
        fs_copy_entry.path.clear();
        fs_copy_entry.filename.clear();
        fs_copy_entry.isDir = false;
    }
    
    void Copy(SceIoDirent &entry, const std::string &path) {
        FS::ClearCopyData();
        fs_copy_entry.path = FS::BuildPath(path, entry.d_name);
        fs_copy_entry.filename.append(entry.d_name);
        
        if (FIO_S_ISDIR(entry.d_stat.st_mode)) {
            fs_copy_entry.isDir = true;
        }
    }
    
    int Paste(void) {
        int ret = 0;
        std::string path = FS::BuildPath(cfg.cwd, fs_copy_entry.filename);
        
        if (fs_copy_entry.isDir) // Copy folder recursively
            ret = FS::CopyDir(fs_copy_entry.path, path);
        else // Copy file
            ret = FS::CopyFile(fs_copy_entry.path, path);
            
        FS::ClearCopyData();
        return ret;
    }
    
    // Thanks to TN for finding this.
    static int sceIoMove(const char *src, const char *dest) {
        int ret = 0;
        size_t i = 0;
        char strage[32] = {0};
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

        u32 data[2] = {0};
        data[0] = (u32)(p1 + 1);
        data[1] = (u32)(p2 + 1);
        
#ifdef FS_DEBUG
        if (R_FAILED(ret = sceIoDevctl(strage, 0x02415830, &data, sizeof(data), nullptr, 0))) {
            Log::Error("sceIoDevctl() failed!", ret);
            return ret;
        }
#else
        if (R_FAILED(ret = pspIoDevctl(strage, 0x02415830, &data, sizeof(data), nullptr, 0))) {
            Log::Error("pspIoDevctl() failed!", ret);
            return ret;
        }
#endif

        return 0;
    }

    int Move(void) {
        int ret = 0;
        std::string path = FS::BuildPath(cfg.cwd, fs_copy_entry.filename);

        if (R_FAILED(ret = sceIoMove(fs_copy_entry.path.c_str(), path.c_str()))) {
            Log::Error("sceIoMove(%s, %s) failed: 0x%x\n", fs_copy_entry.filename.c_str(), path.c_str(), ret);
            FS::ClearCopyData();
            return ret;
        }
        
        FS::ClearCopyData();
        return 0;
    }

    static int DeleteDirectoryRecursive(const std::string &path) {
        int ret = 0;
        SceUID dir = 0;
        scePowerLock(0);

#ifdef FS_DEBUG
        if (R_FAILED(ret = dir = sceIoDopen(path.c_str()))) {
            if (R_FAILED(ret = sceIoRemove(path.c_str()))) {
                Log::Error("sceIoRemove(%s) failed: 0x%08x\n", path.c_str(), ret);
                scePowerUnlock(0);
                return ret;
            }
        }
#else
        if (R_FAILED(ret = dir = pspIoOpenDir(path.c_str()))) {
            if (R_FAILED(ret = pspIoRemoveFile(path.c_str()))) {
                Log::Error("pspIoRemoveFile(%s) failed: 0x%08x\n", path.c_str(), ret);
                scePowerUnlock(0);
                return ret;
            }
        }
#endif

        do {
            if (Utils::IsCancelButtonPressed()) {
#ifdef FS_DEBUG
                sceIoDclose(dir);
#else
                pspIoCloseDir(dir);
#endif
                scePowerUnlock(0);
                return 0;
            }

            SceIoDirent entry;
            std::memset(&entry, 0, sizeof(entry));

#ifdef FS_DEBUG
            ret = sceIoDread(dir, &entry);
#else
            ret = pspIoReadDir(dir, &entry);
#endif
            if (ret > 0) {
                if ((std::strcmp(entry.d_name, ".") == 0) || (std::strcmp(entry.d_name, "..") == 0))
                    continue;

                std::string new_path = FS::BuildPath(path, entry.d_name);
                
                if (FIO_S_ISDIR(entry.d_stat.st_mode)) {
                    int result = FS::DeleteDirectoryRecursive(new_path);
                    if (result <= 0) {
                        Log::Error("FS::DeleteDirectoryRecursive(%s) failed: 0x%08x\n", path.c_str(), ret);
#ifdef FS_DEBUG
                        sceIoDclose(dir);
#else
                        pspIoCloseDir(dir);
#endif
                        scePowerUnlock(0);
                        return ret;
                    }
                }
                else {
#ifdef FS_DEBUG
                    int result = sceIoRemove(new_path.c_str());
                    if (R_FAILED(result)) {
                        Log::Error("sceIoRemove(%s) failed: 0x%08x\n", path.c_str(), ret);
                        sceIoDclose(dir);
#else
                    int result = pspIoRemoveFile(new_path.c_str());
                    if (R_FAILED(result)) {
                        Log::Error("pspIoRemoveFile(%s) failed: 0x%08x\n", path.c_str(), ret);
                        pspIoCloseDir(dir);
#endif
                        scePowerUnlock(0);
                        return ret;
                    }
                }
            }
        } while (ret > 0);
        
#ifdef FS_DEBUG
        sceIoDclose(dir);
#else
        pspIoCloseDir(dir);
#endif
        scePowerUnlock(0);

#ifdef FS_DEBUG
        if (R_FAILED(ret = sceIoRmdir(path.c_str()))) {
            Log::Error("sceIoRmdir(%s) failed: 0x%08x\n", path.c_str(), ret);
            return ret;
        }
#else
        if (R_FAILED(ret = pspIoRemoveDir(path.c_str()))) {
            Log::Error("pspIoRemoveDir(%s) failed: 0x%08x\n", path.c_str(), ret);
            return ret;
        }
#endif

        return 1;
    }

    int Delete(SceIoDirent &entry) {
        int ret = 0;
        std::string path = FS::BuildPath(cfg.cwd, entry.d_name);

        if (FIO_S_ISDIR(entry.d_stat.st_mode)) {
            if (R_FAILED(ret = FS::DeleteDirectoryRecursive(path))) {
                Log::Error("FS::DeleteDirectoryRecursive(%s) failed: 0x%x\n", path.c_str(), ret);
                return ret;
            }
        }
        else {
#ifdef FS_DEBUG
            if (R_FAILED(ret = sceIoRemove(path.c_str()))) {
                Log::Error("sceIoRemove(%s) failed: 0x%x\n", path.c_str(), ret);
                return ret;
            }
#else
            if (R_FAILED(ret = pspIoRemoveFile(path.c_str()))) {
                Log::Error("pspIoRemoveFile(%s) failed: 0x%x\n", path.c_str(), ret);
                return ret;
            }
#endif
        }

        return 0;
    }
    
    std::string BuildPath(const std::string &path, const std::string &filename) {
        std::string result;
        result.reserve(path.size() + 1 + filename.size());
    
        result += path;
    
        if (!path.empty() && path.back() != '/') {
            result += '/';
        }
        
        result += filename;
        return result;
    }
}
