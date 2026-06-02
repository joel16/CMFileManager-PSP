#pragma once

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
    bool FileExists(const char *path);
    bool DirExists(const char *path);
    int MakeDir(const std::string &path);
    int RecursiveMakeDir(const std::string &path);
    int CreateFile(const char *path);
    const char* GetFileExt(const char *filename);
    FileType GetFileType(const char *filename);
    SceOff GetFileSize(const char *path);
    char *GetFileTimestamp(SceIoStat &stat, FileTimestamp time);
    char *GetFilePermission(SceIoStat &stat);
    int ReadFile(const std::string &path, void *buf, int size);
    int WriteFile(const std::string &path, void *buf, int size);
    int GetDirList(const std::string &path, std::vector<SceIoDirent> &entries);
    int ChangeDirNext(const std::string &path, std::vector<SceIoDirent> &entries);
    int ChangeDirPrev(std::vector<SceIoDirent> &entries);
    std::string GetFilename(const std::string &path);
    void Copy(SceIoDirent &entry, const std::string &path);
    int Paste(void);
    int Move(void);
    int Delete(SceIoDirent &entry);
    std::string BuildPath(const std::string &path, const std::string &filename);
}
