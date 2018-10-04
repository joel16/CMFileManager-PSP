#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "fs.h"
#include "utils.h"

bool FS_FileExists(const char *path) {
	SceUID file = 0;
	
	if (R_SUCCEEDED(file = sceIoOpen(path, PSP_O_RDONLY, 0777))) {
		sceIoClose(file);
		return true;
	}
	
	return false;
}

bool FS_DirExists(const char *path) {	
	SceUID dir = 0;
	
	if (R_SUCCEEDED(dir = sceIoDopen(path))) {
		sceIoDclose(dir);
		return true;
	}
	
	return false;
}

int FS_MakeDir(const char *path) {
	int ret = 0;

	if (R_FAILED(ret = sceIoMkdir(path, 0777)))
		return ret;

	return 0;
}

void FS_RecursiveMakeDir(const char *path) {
	char tmp[256];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp), "%s", path);
	len = strlen(tmp);

	if (tmp[len - 1] == '/')
		tmp[len - 1] = 0;

	for (p = tmp + 1; *p; p++) {
		if (*p == '/') {
			*p = 0;
			FS_MakeDir(tmp);
			*p = '/';
		}
		FS_MakeDir(tmp);
	}
}

const char *FS_GetFileExt(const char *filename) {
	const char *dot = strrchr(filename, '.');
	
	if (!dot || dot == filename)
		return "";
	
	return dot + 1;
}

SceOff FS_GetFileSize(const char *path) {
	SceIoStat stat;
	int ret = 0;

	if (R_FAILED(ret = sceIoGetstat(path, &stat)))
		return ret;

	return stat.st_size;
}

ScePspDateTime FS_GetFileModifiedTime(const char *path) {
	SceIoStat stat;
	sceIoGetstat(path, &stat);
	
	return stat.st_mtime;
}
