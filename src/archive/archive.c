#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#include "archive.h"
#include "common.h"
#include "fs.h"
#include "progress_bar.h"
#include "unzip.h"
#include "utils.h"

static const char *Archive_GetFilenameWithoutDir(const char *filename) {
	if (!filename)
		return 0;
	char *p = strrchr(filename, '/');
	if (!p)
		return filename;
	if (p[1] == '\0')
		return 0;
	return p + 1;
}

static int unzExtractCurrentFile(unzFile *unzHandle, int *path) {
	int res = 0;
	char filename[256];
	unsigned int bufsize = (64 * 1024);

	unz_file_info file_info;
	if ((res = unzGetCurrentFileInfo(unzHandle, &file_info, filename, sizeof(filename), NULL, 0, NULL, 0)) != 0) {
		unzClose(unzHandle);
		return -1;
	}

	void *buf = (void *)malloc(bufsize);
	if (!buf)
		return -2;

	char *filenameWithoutPath = Utils_Basename(filename);

	if ((*filenameWithoutPath) == '\0') {
		if ((*path) == 0)
			mkdir(filename, 0777);
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
			return res;
		}

		FILE *out = fopen(write, "wb");

		if ((out == NULL) && ((*path) == 0) && (filenameWithoutPath != (char *)filename)) {
			char c = *(filenameWithoutPath - 1);
			*(filenameWithoutPath - 1) = '\0';
			mkdir(write, 0777);
			*(filenameWithoutPath - 1) = c;
			out = fopen(write, "wb");
		}

		do {
			res = unzReadCurrentFile(unzHandle, buf, bufsize);

			if (res < 0)
				break;

			if (res > 0)
				fwrite(buf, 1, res, out);
		} 
		while (res > 0);

		fclose(out);

		res = unzCloseCurrentFile(unzHandle);
	}
	
	if (buf)
		free(buf);
	
	return res;
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
		return res;
	}
	
	for (i = 0; i < global_info.number_entry; i++) {
		ProgressBar_DisplayProgress("Extracting", filename, i, global_info.number_entry);

		if ((res = unzExtractCurrentFile(unzHandle, &path)) != UNZ_OK)
			break;

		if ((i + 1) < global_info.number_entry) {
			if ((res = unzGoToNextFile(unzHandle)) != UNZ_OK) {// Could not read next file.
				unzClose(unzHandle);
				return res;
			}
		}
	}

	return res;
}

int Archive_ExtractZIP(const char *src, const char *dst) {
	unzFile *unzHandle = unzOpen(src); // Open zip file

	if (unzHandle == NULL) // not found
		return -1;

	int res = unzExtractAll(src, unzHandle);
	res = unzClose(unzHandle);

	return res;
}
