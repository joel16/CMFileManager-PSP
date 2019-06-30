#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "glib2d_helper.h"
#include "menu_archive.h"
#include "menu_audio.h"
#include "menu_error.h"
#include "menu_gallery.h"
#include "textures.h"
#include "utils.h"

int position = 0;   // menu position
int fileCount = 0;  // file count
File *files = NULL; // file list

void Dirbrowse_RecursiveFree(File *node) {
	if (node == NULL) // End of list
		return;
	
	Dirbrowse_RecursiveFree(node->next); // Nest further
	free(node); // Free memory
}

static int cmpstringp(const void *p1, const void *p2) {
	SceIoDirent *entryA = (SceIoDirent *)p1;
	SceIoDirent *entryB = (SceIoDirent *)p2;

	if ((FIO_S_ISDIR(entryA->d_stat.st_mode)) && !(FIO_S_ISDIR(entryB->d_stat.st_mode)))
		return -1;
	else if (!(FIO_S_ISDIR(entryA->d_stat.st_mode)) && (FIO_S_ISDIR(entryB->d_stat.st_mode)))
		return 1;
	else {
		if (config.sort == 0) // Sort alphabetically (ascending - A to Z)
			return strcasecmp(entryA->d_name, entryB->d_name);
		else if (config.sort == 1) // Sort alphabetically (descending - Z to A)
			return strcasecmp(entryB->d_name, entryA->d_name);
		else if (config.sort == 2) // Sort by file size (largest first)
			return entryA->d_stat.st_size > entryB->d_stat.st_size ? -1 : entryA->d_stat.st_size < entryB->d_stat.st_size ? 1 : 0;
		else if (config.sort == 3) // Sort by file size (smallest first)
			return entryB->d_stat.st_size > entryA->d_stat.st_size ? -1 : entryB->d_stat.st_size < entryA->d_stat.st_size ? 1 : 0;
	}

	return 0;
}

int Dirbrowse_PopulateFiles(bool refresh) {
	SceUID dir = 0;
	Dirbrowse_RecursiveFree(files);
	files = NULL;
	fileCount = 0;

	if (R_SUCCEEDED(dir = sceIoDopen(cwd))) {
		int entryCount = 0, i = 0;
		SceIoDirent *entries = (SceIoDirent *)calloc(MAX_FILES, sizeof(SceIoDirent));

		while (sceIoDread(dir, &entries[entryCount]) > 0)
			entryCount++;

		sceIoDclose(dir);
		qsort(entries, entryCount, sizeof(SceIoDirent), cmpstringp);

		for (i = 0; i < entryCount; i++) {
			// Ingore null filename
			if (entries[i].d_name[0] == '\0') 
				continue;

			// Ignore "." in all directories
			if (!strcmp(entries[i].d_name, ".")) 
				continue;

			// Ignore ".." in Root Directory
			if ((!strcmp(cwd, root_path)) && (!strncmp(entries[i].d_name, "..", 2))) // Ignore ".." in Root Directory
				continue;

			// Allocate Memory
			File *item = (File *)malloc(sizeof(File));
			memset(item, 0, sizeof(File));

			// Copy File Name
			strcpy(item->name, entries[i].d_name);
			strcpy(item->ext, FS_GetFileExt(item->name));

			item->isDir = FIO_S_ISDIR(entries[i].d_stat.st_mode);

			if (!item->isDir)
				item->size = entries[i].d_stat.st_size;

			// New List
			if (files == NULL) 
				files = item;

			// Existing List
			else {
				File *list = files;
					
				while(list->next != NULL) 
					list = list->next;

				list->next = item;
			}

			fileCount++;
		}

		free(entries);
	}
	else {
		Menu_DisplayError("sceIoDopen() failed!", dir);
		return dir;
	}

	if (!refresh) {
		if (position >= fileCount) 
			position = fileCount - 1; // Keep index
	}
	else 
		position = 0; // Refresh position

	return 0;
}

void Dirbrowse_DisplayFiles(void) {
	intraFontSetStyle(font, 0.7f, WHITE, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
	intraFontPrint(font, 40, 20 + ((40 - (font->texYSize - 30)) / 2), cwd);

	if (is_ms_inserted) {
		G2D_DrawRect(40, 52, 400, 3, config.dark_theme? SELECTOR_COLOUR_DARK : G2D_RGBA(10, 73, 163, 255));
		G2D_DrawRect(40, 52, (((double)used_storage/(double)total_storage) * 400.0), 3, config.dark_theme? TITLE_COLOUR_DARK : G2D_RGBA(49, 161, 224, 255));
	}

	int i = 0, printed = 0;
	File *file = files; // Draw file list

	for(; file != NULL; file = file->next) {
		if (printed == FILES_PER_PAGE) // Limit the files per page
			break;

		if (position < FILES_PER_PAGE || i > (position - FILES_PER_PAGE)) {
			if (i == position)
				G2D_DrawRect(0, 62 + (42 * printed), 480, 42, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

			// Do not allow parent dir to be multi-selected
			if (strncmp(file->name, "..", 2)) {
				if (strcmp(multi_select_dir, cwd) == 0) {
					multi_select[i] == true? G2D_DrawImage(config.dark_theme? icon_check_dark : icon_check, 5, 71 + (42 * printed)) : 
						G2D_DrawImage(config.dark_theme? icon_uncheck_dark : icon_uncheck, 5, 71 + (42 * printed));
				}
				else
					G2D_DrawImage(config.dark_theme? icon_uncheck_dark : icon_uncheck, 5, 71 + (42 * printed));
			}

			if (file->isDir)
				G2D_DrawImage(config.dark_theme? icon_dir_dark : icon_dir, 34, 65 + (42 * printed));
			else if (!strncasecmp(file->ext, "pbp", 3))
				G2D_DrawImage(icon_app, 34, 65 + (42 * printed));
			else if ((!strncasecmp(file->ext, "flac", 4)) || (!strncasecmp(file->ext, "it", 2)) || (!strncasecmp(file->ext, "mod", 3))
				|| (!strncasecmp(file->ext, "mp3", 3)) || (!strncasecmp(file->ext, "ogg", 3)) || (!strncasecmp(file->ext, "opus", 4))
				|| (!strncasecmp(file->ext, "s3m", 3)) || (!strncasecmp(file->ext, "wav", 3)) || (!strncasecmp(file->ext, "xm", 2)))
				G2D_DrawImage(icon_audio, 34, 65 + (42 * printed));
			else if ((!strncasecmp(file->ext, "7z", 2)) || (!strncasecmp(file->ext, "ar", 2)) || (!strncasecmp(file->ext, "cpio", 4))
				|| (!strncasecmp(file->ext, "grz", 3)) || (!strncasecmp(file->ext, "iso", 3)) || (!strncasecmp(file->ext, "lrz", 3))
				|| (!strncasecmp(file->ext, "mtree", 5)) || (!strncasecmp(file->ext, "rar", 3)) || (!strncasecmp(file->ext, "shar", 4))
				|| (!strncasecmp(file->ext, "tar", 3)) || (!strncasecmp(file->ext, "taz", 3)) || (!strncasecmp(file->ext, "tbz", 3))
				|| (!strncasecmp(file->ext, "tgz", 3)) || (!strncasecmp(file->ext, "tlz", 3)) || (!strncasecmp(file->ext, "txz", 3))
				|| (!strncasecmp(file->ext, "tz", 2)) || (!strncasecmp(file->ext, "tz2", 3)) || (!strncasecmp(file->ext, "tzma", 4))
				|| (!strncasecmp(file->ext, "tzo", 3)) || (!strncasecmp(file->ext, "tzst", 4)) || (!strncasecmp(file->ext, "uu", 2))
				|| (!strncasecmp(file->ext, "war", 3)) || (!strncasecmp(file->ext, "xar", 3)) || (!strncasecmp(file->ext, "zip", 3))
				|| (!strncasecmp(file->ext, "zst", 3)))
				G2D_DrawImage(icon_archive, 34, 65 + (42 * printed));
			else if ((!strncasecmp(file->ext, "bmp", 3)) || (!strncasecmp(file->ext, "gif", 3)) || (!strncasecmp(file->ext, "jpg", 3))
				|| (!strncasecmp(file->ext, "jpeg", 4)) || (!strncasecmp(file->ext, "pcx", 3)) || (!strncasecmp(file->ext, "png", 3))
				|| (!strncasecmp(file->ext, "pgm", 3)) || (!strncasecmp(file->ext, "ppm", 3)) || (!strncasecmp(file->ext, "tga", 3)))
				G2D_DrawImage(icon_image, 34, 65 + (42 * printed));
			else if (!strncasecmp(file->ext, "prx", 3))
				G2D_DrawImage(icon_prx, 34, 65 + (42 * printed));
			else if ((!strncasecmp(file->ext, "cfg", 3)) || (!strncasecmp(file->ext, "log", 3)) || (!strncasecmp(file->ext, "txt", 3)))
				G2D_DrawImage(icon_text, 34, 65 + (42 * printed));
			else
				G2D_DrawImage(icon_file, 34, 65 + (42 * printed));

			char size[16];

			intraFontSetStyle(font, 0.6f, config.dark_theme? WHITE : BLACK, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
			if (!file->isDir) {
				Utils_GetSizeString(size, file->size);
				intraFontPrint(font, 475 - intraFontMeasureText(font, size), 95 + (42 * printed), size);
			}
			
			intraFontSetStyle(font, 0.7f, config.dark_theme? WHITE : BLACK, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
			if (strncmp(file->name, "..", 2) == 0)
				intraFontPrint(font, 80, 62 + ((42 - (font->texYSize - 32)) / 2) + (42 * printed), "Parent folder");
			else 
				intraFontPrint(font, 80, 62 + ((42 - (font->texYSize - 32)) / 2) + (42 * printed), file->name);

			printed++; // Increase printed counter
		}

		i++; // Increase counter
	}
}

static void Dirbrowse_SaveLastDirectory(void) {
	if ((BROWSE_STATE == BROWSE_STATE_INTERNAL) || (BROWSE_STATE == BROWSE_STATE_SD)) {
		char *buf = malloc(256);
		int len = snprintf(buf, 256, "%s\n", cwd);
		FS_WriteFile("lastdir.txt", buf, len);
		free(buf);
	}
}

File *Dirbrowse_GetFileIndex(int index) {
	int i = 0;
	File *file = files; // Find file Item
	
	for(; file != NULL && i != index; file = file->next)
		i++;

	return file; // Return file
}

void Dirbrowse_OpenFile(void) {
	char path[512];
	File *file = Dirbrowse_GetFileIndex(position);

	if (file == NULL)
		return;

	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);

	if (file->isDir) {
		// Attempt to navigate to target
		if (R_SUCCEEDED(Dirbrowse_Navigate(false))) {
			Dirbrowse_SaveLastDirectory();
			Dirbrowse_PopulateFiles(true);
		}
	}
	else if ((!strncasecmp(file->ext, "bmp", 3)) || (!strncasecmp(file->ext, "gif", 3)) || (!strncasecmp(file->ext, "jpg", 3))
		|| (!strncasecmp(file->ext, "jpeg", 4)) || (!strncasecmp(file->ext, "pcx", 3)) || (!strncasecmp(file->ext, "png", 3))
		|| (!strncasecmp(file->ext, "pgm", 3)) || (!strncasecmp(file->ext, "ppm", 3)) || (!strncasecmp(file->ext, "tga", 3)))
		Gallery_DisplayImage(path);
	else if (!strncasecmp(file->ext, "pbp", 3))
		Utils_LaunchEboot(path);
	else if ((!strncasecmp(file->ext, "flac", 4)) || (!strncasecmp(file->ext, "it", 2)) || (!strncasecmp(file->ext, "mod", 3))
		|| (!strncasecmp(file->ext, "mp3", 3)) || (!strncasecmp(file->ext, "ogg", 3)) || (!strncasecmp(file->ext, "opus", 4))
		|| (!strncasecmp(file->ext, "s3m", 3)) || (!strncasecmp(file->ext, "wav", 3)) || (!strncasecmp(file->ext, "xm", 2)))
		Menu_PlayAudio(path);
	else if ((!strncasecmp(file->ext, "7z", 2)) || (!strncasecmp(file->ext, "ar", 2)) || (!strncasecmp(file->ext, "cpio", 4))
		|| (!strncasecmp(file->ext, "grz", 3)) || (!strncasecmp(file->ext, "iso", 3)) || (!strncasecmp(file->ext, "lrz", 3))
		|| (!strncasecmp(file->ext, "mtree", 5)) || (!strncasecmp(file->ext, "rar", 3)) || (!strncasecmp(file->ext, "shar", 4))
		|| (!strncasecmp(file->ext, "tar", 3)) || (!strncasecmp(file->ext, "taz", 3)) || (!strncasecmp(file->ext, "tbz", 3))
		|| (!strncasecmp(file->ext, "tgz", 3)) || (!strncasecmp(file->ext, "tlz", 3)) || (!strncasecmp(file->ext, "txz", 3))
		|| (!strncasecmp(file->ext, "tz", 2)) || (!strncasecmp(file->ext, "tz2", 3)) || (!strncasecmp(file->ext, "tzma", 4))
		|| (!strncasecmp(file->ext, "tzo", 3)) || (!strncasecmp(file->ext, "tzst", 4)) || (!strncasecmp(file->ext, "uu", 2))
		|| (!strncasecmp(file->ext, "war", 3)) || (!strncasecmp(file->ext, "xar", 3)) || (!strncasecmp(file->ext, "zip", 3))
		|| (!strncasecmp(file->ext, "zst", 3))) {
		if (R_SUCCEEDED(Archive_ExtractFile(path)))
			Dirbrowse_PopulateFiles(true);
	}
}

// Navigate to Folder
int Dirbrowse_Navigate(bool parent) {
	File *file = Dirbrowse_GetFileIndex(position); // Get index
	
	if (file == NULL)
		return -1;

	// Special case ".."
	if ((parent) || (!strncmp(file->name, "..", 2))) {
		char *slash = NULL;

		// Find last '/' in working directory
		int i = strlen(cwd) - 2; for(; i >= 0; i--) {
			// Slash discovered
			if (cwd[i] == '/') {
				slash = cwd + i + 1; // Save pointer
				break; // Stop search
			}
		}

		slash[0] = 0; // Terminate working directory
	}

	// Normal folder
	else {
		if (file->isDir) {
			// Append folder to working directory
			strcpy(cwd + strlen(cwd), file->name);
			cwd[strlen(cwd) + 1] = 0;
			cwd[strlen(cwd)] = '/';
		}
	}

	Dirbrowse_SaveLastDirectory();

	return 0; // Return success
}
