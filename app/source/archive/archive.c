#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#include "archive.h"
#include "common.h"
#include "fs.h"
#include "menu_error.h"
#include "progress_bar.h"
#include "unzip.h"
#include "utils.h"

static char *Archive_RemoveFileExt(char *filename) {
	char *ret, *lastdot;

   	if (filename == NULL)
   		return NULL;
   	if ((ret = malloc(strlen(filename) + 1)) == NULL)
   		return NULL;

   	strcpy(ret, filename);
   	lastdot = strrchr(ret, '.');

   	if (lastdot != NULL)
   		*lastdot = '\0';

   	return ret;
}

static int unzExtractCurrentFile(unzFile *unzHandle, int *path) {
	int res = 0;
	char filename[256];
	unsigned int bufsize = (64 * 1024);

	unz_file_info file_info;
	if ((res = unzGetCurrentFileInfo(unzHandle, &file_info, filename, sizeof(filename), NULL, 0, NULL, 0)) != 0) {
		unzClose(unzHandle);
		Menu_DisplayError("unzGetCurrentFileInfo failed:", res);
		return res;
	}

	void *buf = malloc(bufsize);
	if (!buf)
		return -1;

	char *filenameWithoutPath = Utils_Basename(filename);

	if ((*filenameWithoutPath) == '\0') {
		if ((*path) == 0)
			FS_MakeDir(filename);
	}
	else {
		const char *write;

		if ((*path) == 0)
			write = filename;
		else
			write = filenameWithoutPath;
		
		if ((res = unzOpenCurrentFile(unzHandle)) != UNZ_OK) {
			unzClose(unzHandle);
			free(buf);
			Menu_DisplayError("unzOpenCurrentFile failed:", res);
			return res;
		}

		SceUID file = 0;
		file = sceIoOpen(write, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

		if ((file < 0) && ((*path) == 0) && (filenameWithoutPath != (char *)filename)) {
			char c = *(filenameWithoutPath - 1);
			*(filenameWithoutPath - 1) = '\0';
			FS_MakeDir(write);
			*(filenameWithoutPath - 1) = c;
			file = sceIoOpen(write, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		}

		do {
			res = unzReadCurrentFile(unzHandle, buf, bufsize);

			if (res < 0)
				break;

			if (res > 0) {
				sceIoWrite(file, buf, res);
			}
		} 
		while (res > 0);

		sceIoClose(file);

		if ((res = unzCloseCurrentFile(unzHandle)) != UNZ_OK) {
			free(buf);
			Menu_DisplayError("unzCloseCurrentFile failed:", res);
			return res;
		}
	}
	
	if (buf)
		free(buf);
	
	return 0;
}

static int unzExtractAll(const char *src, unzFile *unzHandle) {
	int res = 0;
	int path = 0;
	unsigned int i = 0;
	char *filename = Utils_Basename(src);
	
	unz_global_info global_info;
	memset(&global_info, 0, sizeof(unz_global_info));
	
	if ((res = unzGetGlobalInfo(unzHandle, &global_info)) != UNZ_OK) {// Get info about the zip file
		unzClose(unzHandle);
		Menu_DisplayError("unzGetGlobalInfo failed:", res);
		return res;
	}
	
	for (i = 0; i < global_info.number_entry; i++) {
		ProgressBar_DisplayProgress("Extracting", filename, i, global_info.number_entry);

		if ((res = unzExtractCurrentFile(unzHandle, &path)) != UNZ_OK)
			break;

		if ((i + 1) < global_info.number_entry) {
			if ((res = unzGoToNextFile(unzHandle)) != UNZ_OK) {// Could not read next file.
				unzClose(unzHandle);
				Menu_DisplayError("unzGoToNextFile failed:", res);
				return res;
			}
		}
	}

	return res;
}

int Archive_ExtractZIP(const char *src) {
	char *path = malloc(256);
	char *dirname_without_ext = Archive_RemoveFileExt((char *)src);

	snprintf(path, 512, "%s", dirname_without_ext);
	FS_MakeDir(path);
	sceIoChdir(path);

	unzFile *unzHandle = unzOpen(src); // Open zip file

	if (unzHandle == NULL) {// not found
		free(path);
		free(dirname_without_ext);
		return -1;
	}

	int res = unzExtractAll(src, unzHandle);
	
	if ((res = unzClose(unzHandle)) != UNZ_OK) {
		Menu_DisplayError("unzClose failed:", res);
		return res;
	}

	sceIoChdir(Utils_IsEF0()? "ef0:/PSP/GAME/CMFileManager" : "ms0:/PSP/GAME/CMFileManager");
	return res;
}
