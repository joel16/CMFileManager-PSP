#pragma once

#include <pspiofilemgr.h>
#include <stdbool.h>

// PRX function prototypes
extern int pspOpenDir(const char *dirname);
extern int pspReadDir(SceUID dir, SceIoDirent *dirent);
extern int pspCloseDir(SceUID dir);

bool FS_FileExists(const char *path);
bool FS_DirExists(const char *path);
int FS_MakeDir(const char *path);
void FS_RecursiveMakeDir(const char *path);
int FS_CreateFile(const char *path);
const char *FS_GetFileExt(const char *filename);
SceOff FS_GetFileSize(const char *path);
char *FS_GetFileTimestamp(const char *path, int time);
int FS_ReadFile(char *path, void *buf, int size);
int FS_WriteFile(char *path, void *buf, int size);
char *FS_GetFilePermission(char *path);
int FS_StorageGetMaxSize(const char *dev, u64 *size);
int FS_StorageGetFreeSize(const char *dev, u64 *size);
