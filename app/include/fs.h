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

namespace FS {
    bool FileExists(const std::string &path);
    bool DirExists(const std::string &path);
    int MakeDir(const std::string &path);
    int CreateFile(const std::string &path);
    std::string GetFileExt(const std::string &filename);
    FileType GetFileType(const std::string &filename);
    SceOff GetFileSize(const std::string &path);
    int ReadFile(const std::string &path, void *buf, int size);
    int WriteFile(const std::string &path, void *buf, int size);
    int GetDirList(const std::string &path, std::vector<SceIoDirent> &entries);
    int ChangeDirNext(const std::string &path, std::vector<SceIoDirent> &entries);
    int ChangeDirPrev(std::vector<SceIoDirent> &entries);
    void Copy(SceIoDirent *entry, const std::string &path);
    int Paste(void);
    int Move(void);
    int Delete(SceIoDirent *entry);
}

#endif
