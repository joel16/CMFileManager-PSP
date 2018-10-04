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

char *FS_GetFileTimestamp(const char *path, int time) {
	static char timeStr[20];
	SceIoStat stat;

	if (R_FAILED(sceIoGetstat(path, &stat)))
		return NULL;

	switch(time) {
		case 0:
			snprintf(timeStr, sizeof(timeStr), "%d/%d/%d %2i:%02i", stat.st_ctime.year, stat.st_ctime.month, stat.st_ctime.day, stat.st_ctime.hour, stat.st_ctime.minute);
			break;
		case 1:
			snprintf(timeStr, sizeof(timeStr), "%d/%d/%d %2i:%02i", stat.st_atime.year, stat.st_atime.month, stat.st_atime.day, stat.st_atime.hour, stat.st_atime.minute);
			break;
		case 2:
			snprintf(timeStr, sizeof(timeStr), "%d/%d/%d %2i:%02i", stat.st_mtime.year, stat.st_mtime.month, stat.st_mtime.day, stat.st_mtime.hour, stat.st_mtime.minute);
			break;
	}

	return timeStr;
}

int FS_ReadFile(char *path, void *buf, int size) {
	SceUID file = 0;

	if (R_SUCCEEDED(file = sceIoOpen(path, PSP_O_RDONLY, 0))) {
		int read = sceIoRead(file, buf, size);
		sceIoClose(file);
		return read;
	}
	
	return file;
}

int FS_WriteFile(char *path, void *buf, int size) {	
	SceUID file = 0;
	
	if (R_SUCCEEDED(file = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777))) {
		int written = sceIoWrite(file, buf, size);
		sceIoClose(file);
		return written;
	}
		
	return file;
}
