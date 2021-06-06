#include <archive.h>
#include <archive_entry.h>
#include <filesystem>
#include <psppower.h>
#include <string>

#include "colours.h"
#include "config.h"
#include "fs.h"
#include "g2d.h"
#include "gui.h"
#include "log.h"
#include "textures.h"
#include "utils.h"

int chmod(const char *pathname, mode_t mode) {
    return 0;
}

namespace ArchiveHelper {
    u64 CountFiles(const std::string &path) {
        int ret = 0;
        u64 count = 0;

        struct archive *arch = archive_read_new();
        archive_read_support_format_all(arch);

        if ((ret = archive_read_open_filename(arch, path.c_str(), 0x3000)) != ARCHIVE_OK) {
            archive_read_close(arch);
            archive_read_free(arch);
            return ret;
        }

        struct archive_entry *entry;
        while((ret = archive_read_next_header(arch, &entry)) == ARCHIVE_OK) {
            if (ret == ARCHIVE_EOF)
                break;

            count++;
        }
        
        archive_read_close(arch);
        archive_read_free(arch);
        return count;
    }

    static int CopyData(struct archive *src, struct archive *dest) {
        int ret = 0;
        const void *buff = nullptr;
        size_t size = 0;
        int64_t offset = 0;
        
        for (;;) {
            ret = archive_read_data_block(src, &buff, &size, &offset);
            if (ret == ARCHIVE_EOF)
                return ARCHIVE_OK;
            if (ret != ARCHIVE_OK)
                return ret;
                
            ret = archive_write_data_block(dest, buff, size, offset);
            if (ret != ARCHIVE_OK)
                return ret;
        }

        return 0;
    }

    static void FreeHandles(struct archive *read, struct archive *write) {
        archive_read_close(read);
        archive_read_free(read);
        archive_write_close(write);
        archive_write_free(write);
        scePowerUnlock(0);
    }

    int Extract(const std::string &path) {
        int ret = 0;
        scePowerLock(0);

        int flags = ARCHIVE_EXTRACT_TIME;
        flags |= ARCHIVE_EXTRACT_PERM;
        flags |= ARCHIVE_EXTRACT_ACL;
        flags |= ARCHIVE_EXTRACT_FFLAGS;

        struct archive *arch = archive_read_new();
        archive_read_support_format_all(arch);

        struct archive *ext = archive_write_disk_new();
        archive_write_disk_set_options(ext, flags);

        struct archive_entry *entry = nullptr;

        if ((ret = archive_read_open_filename(arch, path.c_str(), 0x3000)) != ARCHIVE_OK) {
            Log::Error("archive_read_open_filename(%s) failed: %s\n", path.c_str(), archive_error_string(arch));
            ArchiveHelper::FreeHandles(arch, ext);
            return ret;
        }

        u64 index = 0, count = ArchiveHelper::CountFiles(path);
        std::string filename = std::filesystem::path(path).filename();
        std::string dest = cfg.cwd;
        dest.append("/");
        dest.append(std::filesystem::path(path).stem());
        FS::MakeDir(dest);

        for (;;) {
            if (Utils::IsCancelButtonPressed()) {
                ArchiveHelper::FreeHandles(arch, ext);
                return 0;
            }

            ret = archive_read_next_header(arch, &entry);
            if (ret == ARCHIVE_EOF)
                break;
                
            if (ret != ARCHIVE_OK) {
                Log::Error("archive_read_next_header(%s) failed: %s\n", path.c_str(), archive_error_string(arch));
                ArchiveHelper::FreeHandles(arch, ext);
                return ret;
            }
            
            const char *entry_name = archive_entry_pathname(entry);
            std::string dest_path = dest + "/";
            dest_path.append(entry_name);
            
            archive_entry_update_pathname_utf8(entry, dest_path.c_str());
            ret = archive_write_header(ext, entry);
            
            if (ret < ARCHIVE_OK) {
                Log::Error("archive_write_header(%s) failed: %s\n", dest_path.c_str(), archive_error_string(arch));
                ArchiveHelper::FreeHandles(arch, ext);
                return ret;
            }
            else if (archive_entry_size(entry) > 0) {
                ret = ArchiveHelper::CopyData(arch, ext);
                if (ret != ARCHIVE_OK) {
                    Log::Error("ArchiveHelper::CopyData(%s) failed: %s\n", dest_path.c_str(), archive_error_string(arch));
                    ArchiveHelper::FreeHandles(arch, ext);
                    return ret;
                }
            }
            
            archive_write_finish_entry(ext);
            GUI::ProgressBar("Extracting", filename, index, count);
            index++;
        }
        
        ArchiveHelper::FreeHandles(arch, ext);
        return 0;
    }
}
