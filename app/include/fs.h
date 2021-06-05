#ifndef _CMFILEMANAGER_FS_H_
#define _CMFILEMANAGER_FS_H_

#include <pspiofilemgr.h>
#include <string>
#include <vector>

typedef enum FileType {
    FileTypeNone,
    FileTypeApp,
    FileTypeArchive,
    FileTypeAudio,
    FileTypeImage,
    FileTypeText
} FileType;

typedef enum FileTimestamp {
    FileCreatedTime,
    FileAccessedTime,
    FileModifiedTime
} FileTimestamp;

namespace FS {
    bool FileExists(const std::string &path);
    bool DirExists(const std::string &path);
    int MakeDir(const std::string &path);
    int CreateFile(const std::string &path);
    std::string GetFileExt(const std::string &filename);
    FileType GetFileType(const std::string &filename);
    SceOff GetFileSize(const std::string &path);
    char *GetFileTimestamp(SceIoStat *stat, FileTimestamp time);
    char *GetFilePermission(SceIoStat *stat);
    int ReadFile(const std::string &path, void *buf, int size);
    int WriteFile(const std::string &path, void *buf, int size);
    int GetDirList(const std::string &path, std::vector<SceIoDirent> &entries);
    int ChangeDirNext(const std::string &path, std::vector<SceIoDirent> &entries);
    int ChangeDirPrev(std::vector<SceIoDirent> &entries);
    std::string GetFilename(const std::string &path);
    void Copy(SceIoDirent *entry, const std::string &path);
    int Paste(void);
    int Move(void);
    int Delete(SceIoDirent *entry);
    std::string BuildPath(const std::string &path, const std::string &filename);
}

#endif
