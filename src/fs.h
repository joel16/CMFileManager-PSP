#pragma once

#include <pspiofilemgr.h>

bool FS_FileExists(const char *path);
bool FS_DirExists(const char *path);
int FS_MakeDir(const char *path);
void FS_RecursiveMakeDir(const char *path);
const char *FS_GetFileExt(const char *filename);
SceOff FS_GetFileSize(const char *path);
ScePspDateTime FS_GetFileModifiedTime(const char *path);
