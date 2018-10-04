#include <stdbool.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
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

int cmpstringp(const void *p1, const void *p2) {
	SceIoDirent* entryA = (SceIoDirent*) p1;
	SceIoDirent* entryB = (SceIoDirent*) p2;

	if ((FIO_S_ISDIR(entryA->d_stat.st_mode)) && !(FIO_S_ISDIR(entryB->d_stat.st_mode)))
		return -1;
	else if (!(FIO_S_ISDIR(entryA->d_stat.st_mode)) && (FIO_S_ISDIR(entryB->d_stat.st_mode)))
		return 1;
	else {
		if (config_sort_by == 0) // Sort alphabetically (ascending - A to Z)
			return strcasecmp(entryA->d_name, entryB->d_name);
		else if (config_sort_by == 1) // Sort alphabetically (descending - Z to A)
			return strcasecmp(entryB->d_name, entryA->d_name);
		else if (config_sort_by == 2) // Sort by file size (largest first)
			return entryA->d_stat.st_size > entryB->d_stat.st_size ? -1 : entryA->d_stat.st_size < entryB->d_stat.st_size ? 1 : 0;
		else if (config_sort_by == 3) // Sort by file size (smallest first)
			return entryB->d_stat.st_size > entryA->d_stat.st_size ? -1 : entryB->d_stat.st_size < entryA->d_stat.st_size ? 1 : 0;
	}

	return 0;
}

int Dirbrowse_PopulateFiles(bool refresh) {
	int dir = 0, i = 0;
	Dirbrowse_RecursiveFree(files);
	files = NULL;
	fileCount = 0;

	if (R_SUCCEEDED(dir = sceIoDopen(cwd))) {
		int entryCount = 0;
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
			if (strcmp(entries[i].d_name, ".") == 0) 
				continue;

			// Ignore ".." in Root Directory
			if (strcmp(cwd, ROOT_PATH) == 0 && strncmp(entries[i].d_name, "..", 2) == 0) // Ignore ".." in Root Directory
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
		sceIoDclose(dir);
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

void Dirbrowse_DisplayFiles(void)
{
	oslIntraFontSetStyle(font, 0.6f, WHITE, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawString(40, 20 + ((40 - (font->charHeight - 6)) / 2), cwd);

	int i = 0, printed = 0;
	File *file = files; // Draw file list

	for(; file != NULL; file = file->next) {
		if (printed == FILES_PER_PAGE) // Limit the files per page
			break;

		if (position < FILES_PER_PAGE || i > (position - FILES_PER_PAGE)) {
			if (i == position)
				oslDrawFillRect(0, 62 + (42 * printed), 480, (62 + (42 * printed) + 42), config_dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

			if (strcmp(multi_select_dir, cwd) == 0) {
				multi_select[i] == true? oslDrawImageXY(config_dark_theme? icon_check_dark : icon_check, 5, 71 + (42 * printed)) : 
					oslDrawImageXY(config_dark_theme? icon_uncheck_dark : icon_uncheck, 5, 71 + (42 * printed));
			}
			else
				oslDrawImageXY(config_dark_theme? icon_uncheck_dark : icon_uncheck, 5, 71 + (42 * printed));

			char path[512];
			strcpy(path, cwd);
			strcpy(path + strlen(path), file->name);

			if (file->isDir)
				oslDrawImageXY(config_dark_theme? icon_dir_dark : icon_dir, 34, 65 + (42 * printed));
			else if (!strncasecmp(file->ext, "pbp", 3))
				oslDrawImageXY(icon_app, 34, 65 + (42 * printed));
			else if ((!strncasecmp(file->ext, "mp3", 3)) || (!strncasecmp(file->ext, "wav", 3)) || (!strncasecmp(file->ext, "flac", 3)))
				oslDrawImageXY(icon_audio, 34, 65 + (42 * printed));
			else if ((!strncasecmp(file->ext, "zip", 3)) || (!strncasecmp(file->ext, "rar", 3)))
				oslDrawImageXY(icon_audio, 34, 65 + (42 * printed));
			else if ((!strncasecmp(file->ext, "iso", 3)) || (!strncasecmp(file->ext, "cso", 3)))
				oslDrawImageXY(icon_cd, 34, 65 + (42 * printed));
			else if ((!strncasecmp(file->ext, "gif", 3)) || (!strncasecmp(file->ext, "jpg", 3)) || (!strncasecmp(file->ext, "png", 3)))
				oslDrawImageXY(icon_image, 34, 65 + (42 * printed));
			else if (!strncasecmp(file->ext, "prx", 3))
				oslDrawImageXY(icon_prx, 34, 65 + (42 * printed));
			else if ((!strncasecmp(file->ext, "cfg", 3)) || (!strncasecmp(file->ext, "log", 3)) || (!strncasecmp(file->ext, "txt", 3)))
				oslDrawImageXY(icon_text, 34, 65 + (42 * printed));
			else
				oslDrawImageXY(icon_file, 34, 65 + (42 * printed));

			char buf[64], size[16];
			strncpy(buf, file->name, sizeof(buf));
			buf[sizeof(buf) - 1] = '\0';

			oslIntraFontSetStyle(font, 0.5f, config_dark_theme? WHITE : BLACK, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
			if (!file->isDir) {
				Utils_GetSizeString(size, file->size);
				oslDrawString(470 - oslGetStringWidth(size), 85 + (42 * printed), size);
			}
			
			oslIntraFontSetStyle(font, 0.6f, config_dark_theme? WHITE : BLACK, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
			if (strncmp(file->name, "..", 2) == 0)
				oslDrawString(80, 62 + ((42 - (font->charHeight - 6)) / 2) + (42 * printed), "Parent folder");
			else 
				oslDrawString(80, 62 + ((42 - (font->charHeight - 6)) / 2) + (42 * printed), buf);

			printed++; // Increase printed counter
		}

		i++; // Increase counter
	}
}

static void Dirbrowse_SaveLastDirectory(void) {
	char *buf = (char *)malloc(512);
	strcpy(buf, cwd);

	FILE *write = fopen("lastdir.txt", "w");
	fprintf(write, "%s", buf);
	fclose(write);
	free(buf);
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
}

// Navigate to Folder
int Dirbrowse_Navigate(bool parent) {
	File *file = Dirbrowse_GetFileIndex(position); // Get index
	
	if (file == NULL)
		return -1;

	// Special case ".."
	if ((parent) || (strncmp(file->name, "..", 2) == 0)) {
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
