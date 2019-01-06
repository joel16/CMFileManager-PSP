#include <stdbool.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "osl_helper.h"
#include "progress_bar.h"
#include "textures.h"
#include "utils.h"

/*
*	Copy Flags
*/
#define COPY_FOLDER_RECURSIVE 2
#define COPY_DELETE_ON_FINISH 1
#define COPY_KEEP_ON_FINISH   0
#define NOTHING_TO_COPY      -1

/*
*	Copy Mode
*	-1 : Nothing
*	0  : Copy
*	1  : Move
*/
static int copymode = NOTHING_TO_COPY;
/*
*	Copy Move Origin
*/
static char copysource[1024];

static int delete_dialog_selection = 0, row = 0, column = 0;
static bool copy_status = false, cut_status = false, options_more = false;

void FileOptions_ResetClipboard(void) {
	multi_select_index = 0;
	memset(multi_select, 0, sizeof(multi_select));
	memset(multi_select_indices, 0, sizeof(multi_select_indices));
	memset(multi_select_dir, 0, sizeof(multi_select_dir));
	memset(multi_select_paths, 0, sizeof(multi_select_paths));
}

static int FileOptions_CreateFolder(void) {
	char *buf = malloc(256);
	OSL_DisplayKeyboard("Enter name", "New folder", buf);

	if (strncmp(buf, "", 1) == 0)
		return -1;

	char path[512];
	strcpy(path, cwd);
	strcat(path, buf);
	free(buf);

	FS_RecursiveMakeDir(path);
	Dirbrowse_PopulateFiles(true);
	options_more = false;
	MENU_STATE = MENU_STATE_HOME;
	return 0;
}

static int FileOptions_CreateFile(void) {
	int ret = 0;
	char *buf = malloc(256);
	OSL_DisplayKeyboard("Enter name", "New file", buf);

	if (strncmp(buf, "", 1) == 0)
		return -1;

	char path[512];
	strcpy(path, cwd);
	strcat(path, buf);
	free(buf);

	if (R_FAILED(ret = FS_CreateFile(path)))
		return ret;

	Dirbrowse_PopulateFiles(true);
	options_more = false;
	MENU_STATE = MENU_STATE_HOME;
	return 0;
}

static int FileOptions_Rename(void) {
	int ret = 0;
	File *file = Dirbrowse_GetFileIndex(position);

	if (file == NULL)
		return -1;

	if (strncmp(file->name, "..", 2) == 0)
		return -2;

	char oldPath[512], newPath[512];

	char *buf = malloc(256);

	strcpy(oldPath, cwd);
	strcpy(newPath, cwd);
	strcat(oldPath, file->name);
	
	OSL_DisplayKeyboard("Enter name", file->name, buf);
	strcat(newPath, buf);
	free(buf);

	if (file->isDir) {
		if (R_FAILED(ret = sceIoRename(oldPath, newPath)))
			return ret;
	}
	else {
		if (R_FAILED(ret = sceIoRename(oldPath, newPath)))
			return ret;
	}

	Dirbrowse_PopulateFiles(true);
	options_more = false;
	MENU_STATE = MENU_STATE_HOME;
	return 0;
}

static int FileOptions_RmdirRecursive(char *path)
{
	SceUID dir = 0;
	int i = 0;
	File *filelist = NULL;

	if (R_SUCCEEDED(dir = sceIoDopen(path))) {
		int entryCount = 0;
		SceIoDirent *entries = (SceIoDirent *)calloc(MAX_FILES, sizeof(SceIoDirent));

		while (sceIoDread(dir, &entries[entryCount]) > 0)
			entryCount++;

		sceIoDclose(dir);
		qsort(entries, entryCount, sizeof(SceIoDirent), Utils_Alphasort);

		for (i = 0; i < entryCount; i++) {
			if (strlen(entries[i].d_name) > 0) {
				if ((!strcmp(entries[i].d_name, ".")) || (!strcmp(entries[i].d_name, "..")))
					continue;

				// Allocate Memory
				File *item = (File *)malloc(sizeof(File));
				memset(item, 0, sizeof(File));

				// Copy File Name
				strcpy(item->name, entries[i].d_name);

				item->isDir = FIO_S_ISDIR(entries[i].d_stat.st_mode);

				// New List
				if (filelist == NULL) 
					filelist = item;

				// Existing List
				else {
					File *list = filelist;
					
					while(list->next != NULL) 
						list = list->next;

					list->next = item;
				}
			}
		}

		free(entries);
	}
	else {
		sceIoDclose(dir);
		return dir;
	}

	// List Node
	File *node = filelist;

	// Iterate Files
	for(; node != NULL; node = node->next) {
		if (node->isDir) {
			// Required Buffer Size
			int size = strlen(path) + strlen(node->name) + 2;
			char *buffer = malloc(size);

			// Combine Path
			strcpy(buffer, path);
			strcpy(buffer + strlen(buffer), node->name);
			buffer[strlen(buffer) + 1] = 0;
			buffer[strlen(buffer)] = '/';

			// Recursion Delete
			FileOptions_RmdirRecursive(buffer);
			free(buffer);
		}
		else {
			// Required Buffer Size
			int size = strlen(path) + strlen(node->name) + 1;
			char *buffer = malloc(size);

			// Combine Path
			strcpy(buffer, path);
			strcpy(buffer + strlen(buffer), node->name);

			// Delete File
			sceIoRemove(buffer);
			free(buffer);
		}
	}

	Dirbrowse_RecursiveFree(filelist);
	path[strlen(path) - 1] = '\0';
	return sceIoRmdir(path);
}

static int FileOptions_DeleteFile(void) {
	File *file = Dirbrowse_GetFileIndex(position);

	if (file == NULL)
		return -1;
	
	if (strncmp(file->name, "..", 2) == 0)
			return -2;

	char path[512];
	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);

	int ret = 0;

	if (file->isDir) {
		// Add Trailing Slash
		path[strlen(path) + 1] = 0;
		path[strlen(path)] = '/';
		if (R_FAILED(ret = FileOptions_RmdirRecursive(path)))
			return ret;
	}
	else {
		if (R_FAILED(ret = sceIoRemove(path)))
			return ret;
	}
	
	return 0;
}

static void HandleDelete(void) {
	scePowerLock(0);
	int i = 0;

	if ((multi_select_index > 0) && (strlen(multi_select_dir) != 0)) {
		for (i = 0; i < multi_select_index; i++) {
			if (strlen(multi_select_paths[i]) != 0) {
				if (strncmp(multi_select_paths[i], "..", 2) != 0) {
					if (FS_DirExists(multi_select_paths[i])) {
						multi_select_paths[i][strlen(multi_select_paths[i]) + 1] = 0;
						multi_select_paths[i][strlen(multi_select_paths[i])] = '/';
						FileOptions_RmdirRecursive(multi_select_paths[i]);
					}
					else if (FS_FileExists(multi_select_paths[i]))
						sceIoRemove(multi_select_paths[i]);
				}
			}
		}

		FileOptions_ResetClipboard();
	}
	else if (R_FAILED(FileOptions_DeleteFile())) {
		scePowerUnlock(0);
		return;
	}

	Dirbrowse_PopulateFiles(true);
	scePowerUnlock(0);
	MENU_STATE = MENU_STATE_HOME;
}

void Menu_ControlDeleteDialog(void) {
	if (osl_keys->pressed.right)
		delete_dialog_selection++;
	else if (osl_keys->pressed.left)
		delete_dialog_selection--;

	Utils_SetMax(&delete_dialog_selection, 0, 1);
	Utils_SetMin(&delete_dialog_selection, 1, 0);

	if (osl_keys->pressed.circle) {
		delete_dialog_selection = 0;
		MENU_STATE = MENU_STATE_FILEOPTIONS;
	}

	if (osl_keys->pressed.cross) {
		if (delete_dialog_selection == 1)
			HandleDelete();
		else
			MENU_STATE = MENU_STATE_FILEOPTIONS;

		delete_dialog_selection = 0;
	}
}

static int sceIoMove(const char *src, const char *dest) {
	int ret = 0;
	size_t i = 0;
	char strage[32];
	char *p1, *p2;
	p1 = strchr(src, ':');

	if (p1 == NULL)
		return -1;

	p2 = strchr(dest, ':');
	if (p2 == NULL)
		return -1;

	if ((p1-src) != (p2-dest))
		return -1;

	for (i = 0; (src+i) <= p1; i++) {
		if ((i+1) >= sizeof(strage))
			return -1;
		
		if (src[i] != dest[i])
			return -1;
		strage[i] = src[i];
	}

	strage[i] = '\0';

	// Thanks to TN for finding this.
	u32 data[2];
	data[0] = (u32)(p1+1);
	data[1] = (u32)(p2+1);

	if (R_FAILED(ret = sceIoDevctl(strage, 0x02415830, &data, sizeof(data), NULL, 0)))
		return ret;

	return 0;
}

static int FileOptions_CopyFile(char *src, char *dst, bool display_anim) {
	int chunksize = (512 * 1024); // Chunk size
	char *buffer = malloc(chunksize); // Reading buffer

	int totalwrite = 0, totalread = 0, ret = 0, input_file = 0, output_file = 0;
	SceOff size = FS_GetFileSize(src);

	if (R_SUCCEEDED(input_file = sceIoOpen(src, PSP_O_RDONLY, 0777))) {
		sceIoRemove(dst); // Delete Output File (if existing)

		if (R_SUCCEEDED(output_file = sceIoOpen(dst, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777)) >= 0) {
			int read = 0;

			// Copy Loop (512KB at a time)
			while((read = sceIoRead(input_file, buffer, chunksize)) > 0) {
				// Accumulate Read Data
				totalread += read;
				totalwrite += sceIoWrite(output_file, buffer, read);

				if (display_anim)
					ProgressBar_DisplayProgress(copymode == 1? "Moving" : "Copying", Utils_Basename(src), totalread, size);
			}

			sceIoClose(output_file);

			// Insufficient Copy
			if (totalread != totalwrite) 
				ret = -3;
		}
		else 
			ret = -2;

		sceIoClose(input_file);
	}
	else 
		ret = -1;

	free(buffer);
	return ret;
}

static int FileOptions_CopyDir(char *src, char *dst) {
	int dir = 0, i = 0;

	if (R_SUCCEEDED(dir = sceIoDopen(src))) {

		// Create Output Directory (is allowed to fail, we can merge folders after all)
		sceIoMkdir(dst, 0777);

		int entryCount = 0;
		SceIoDirent *entries = (SceIoDirent *)calloc(MAX_FILES, sizeof(SceIoDirent));

		while (sceIoDread(dir, &entries[entryCount]) > 0)
			entryCount++;

		sceIoDclose(dir);

		qsort(entries, entryCount, sizeof(SceIoDirent), Utils_Alphasort);

		for (i = 0; i < entryCount; i++) {
			if (strlen(entries[i].d_name) > 0) {

				if ((!strcmp(entries[i].d_name, ".")) || (!strcmp(entries[i].d_name, "..")))
					continue;

				// Calculate Buffer Size
				int insize = strlen(src) + strlen(entries[i].d_name) + 2;
				int outsize = strlen(dst) + strlen(entries[i].d_name) + 2;

				// Allocate Buffer
				char *inbuffer = malloc(insize);
				char *outbuffer = malloc(outsize);

				// Puzzle Input Path
				strcpy(inbuffer, src);
				inbuffer[strlen(inbuffer) + 1] = 0;
				inbuffer[strlen(inbuffer)] = '/';
				strcpy(inbuffer + strlen(inbuffer), entries[i].d_name);

				// Puzzle Output Path
				strcpy(outbuffer, dst);
				outbuffer[strlen(outbuffer) + 1] = 0;
				outbuffer[strlen(outbuffer)] = '/';
				strcpy(outbuffer + strlen(outbuffer), entries[i].d_name);

				// Another Folder
				if (FIO_S_ISDIR(entries[i].d_stat.st_mode))
					FileOptions_CopyDir(inbuffer, outbuffer); // Copy Folder (via recursion)

				// Simple File
				else
					FileOptions_CopyFile(inbuffer, outbuffer, false); // Copy File

				// Free Buffer
				free(inbuffer);
				free(outbuffer);
			}

			ProgressBar_DisplayProgress(copymode == 1? "Moving" : "Copying", Utils_Basename(entries[i].d_name), i, entryCount);
		}

		free(entries);
	}
	else {
		sceIoDclose(dir);
		return dir;
	}

	return 0;
}

static void FileOptions_Copy(int flag) {
	File *file = Dirbrowse_GetFileIndex(position);
	
	if (file == NULL)
		return;

	// Copy file source
	strcpy(copysource, cwd);
	strcpy(copysource + strlen(copysource), file->name);

	if ((file->isDir) && (strncmp(file->name, "..", 2) != 0)) // If directory, add recursive folder flag
		flag |= COPY_FOLDER_RECURSIVE;

	copymode = flag; // Set copy flags
}

// Paste file or folder
static int FileOptions_Paste(void) {
	if (copymode == NOTHING_TO_COPY) // No copy source
		return -1;

	// Source and target folder are identical
	char *lastslash = NULL;
	int i = 0;

	for(; i < strlen(copysource); i++)
		if (copysource[i] == '/')
			lastslash = copysource + i;

	char backup = lastslash[1];
	lastslash[1] = 0;
	int identical = strcmp(copysource, cwd) == 0;
	lastslash[1] = backup;

	if (identical)
		return -2;

	char *filename = lastslash + 1; // Source filename

	int requiredlength = strlen(cwd) + strlen(filename) + 1; // Required target path buffer size
	char *copytarget = malloc(requiredlength); // Allocate target path buffer

	// Puzzle target path
	strcpy(copytarget, cwd);
	strcpy(copytarget + strlen(copytarget), filename);

	int ret = -3; // Return result

	// Recursive folder copy
	if ((copymode & COPY_FOLDER_RECURSIVE) == COPY_FOLDER_RECURSIVE) {
		// Check files in current folder
		File *node = files; 

		for(; node != NULL; node = node->next) {
			if ((strcmp(filename, node->name) == 0) && (!node->isDir)) // Found a file matching the name (folder = ok, file = not)
				return -4; // Error out
		}

		ret = FileOptions_CopyDir(copysource, copytarget); // Copy folder recursively

		if ((R_SUCCEEDED(ret)) && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH) {
			copysource[strlen(copysource) + 1] = 0;
			copysource[strlen(copysource)] = '/';
			FileOptions_RmdirRecursive(copysource); // Delete dir
		}
	}

	// Simple file copy
	else {
		ret = FileOptions_CopyFile(copysource, copytarget, true); // Copy file
		
		if ((R_SUCCEEDED(ret)) && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
			sceIoRemove(copysource); // Delete file
	}

	// Paste success
	if (R_SUCCEEDED(ret)) {
		memset(copysource, 0, sizeof(copysource)); // Erase cache data
		copymode = NOTHING_TO_COPY;
	}

	free(copytarget); // Free target path buffer
	return ret;
}

static void HandleCopy(void) {
	scePowerLock(0);
	int i = 0;

	if ((!copy_status) && (!cut_status)) {
		copy_status = true;
		FileOptions_Copy(COPY_KEEP_ON_FINISH);
		MENU_STATE = MENU_STATE_HOME;
	}
	else if (copy_status) {
		if ((multi_select_index > 0) && (strlen(multi_select_dir) != 0)) {
			char dest[512];
			
			for (i = 0; i < multi_select_index; i++) {
				if (strlen(multi_select_paths[i]) != 0) {
					if (strncmp(multi_select_paths[i], "..", 2) != 0) {
						snprintf(dest, 512, "%s%s", cwd, Utils_Basename(multi_select_paths[i]));
				
						if (FS_DirExists(multi_select_paths[i]))
							FileOptions_CopyDir(multi_select_paths[i], dest);
						else if (FS_FileExists(multi_select_paths[i]))
							FileOptions_CopyFile(multi_select_paths[i], dest, true);
					}
				}
			}
			
			FileOptions_ResetClipboard();
			copymode = NOTHING_TO_COPY;
			
		}
		else if (R_FAILED(FileOptions_Paste())) {
			scePowerUnlock(0);
			return;
		}

		copy_status = false;
		Dirbrowse_PopulateFiles(true);
		MENU_STATE = MENU_STATE_HOME;
	}

	scePowerUnlock(0);
}

static void HandleCut(void) {
	scePowerLock(0);
	int i = 0;

	if ((!cut_status) && (!copy_status)) {
		cut_status = true;
		FileOptions_Copy(COPY_DELETE_ON_FINISH);
		MENU_STATE = MENU_STATE_HOME;
	}
	else if (cut_status) {
		char dest[512];

		if ((multi_select_index > 0) && (strlen(multi_select_dir) != 0)) {
			for (i = 0; i < multi_select_index; i++) {
				if (strlen(multi_select_paths[i]) != 0) {
					snprintf(dest, 512, "%s%s", cwd, Utils_Basename(multi_select_paths[i]));
					
					if (FS_DirExists(multi_select_paths[i]))
						sceIoMove(multi_select_paths[i], dest);
					else if (FS_FileExists(multi_select_paths[i]))
						sceIoMove(multi_select_paths[i], dest);
				}
			}

			FileOptions_ResetClipboard();
		}
		else {
			snprintf(dest, 512, "%s%s", cwd, Utils_Basename(copysource));

			if (FS_DirExists(copysource))
				sceIoMove(copysource, dest);
			else if (FS_FileExists(copysource))
				sceIoMove(copysource, dest);
		}

		cut_status = false;
		copymode = NOTHING_TO_COPY;
		Dirbrowse_PopulateFiles(true);
		MENU_STATE = MENU_STATE_HOME;
	}

	scePowerUnlock(0);
}

void Menu_DisplayDeleteDialog(void) {
	int text_width = oslGetStringWidth("Do you want to continue?");

	oslDrawImageXY(config.dark_theme? dialog_dark : dialog, ((480 - oslGetImageWidth(dialog)) / 2), ((272 - oslGetImageHeight(dialog)) / 2));

	oslIntraFontSetStyle(font, 0.6f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawString(((480 - oslGetImageWidth(dialog)) / 2) + 8, ((272 - oslGetImageHeight(dialog)) / 2) + 6, "Confirm deletion");

	oslIntraFontSetStyle(font, 0.6f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawString(((480 - (text_width)) / 2), ((272 - oslGetImageHeight(dialog)) / 2) + 40, "Do you wish to continue?");

	if (delete_dialog_selection == 0)
		OSL_DrawFillRect((364 - oslGetStringWidth("NO")) - 5, (191 - (font->charHeight - 6)) - 5, oslGetStringWidth("NO") + 10, (font->charHeight - 6) + 10, 
			config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (delete_dialog_selection == 1)
		OSL_DrawFillRect((409 - (oslGetStringWidth("YES"))) - 5, (191 - (font->charHeight - 6)) - 5, oslGetStringWidth("YES") + 10, (font->charHeight - 6) + 10, 
			config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

	// 419
	oslIntraFontSetStyle(font, 0.6f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawString(409 - (oslGetStringWidth("YES")), (191 - (font->charHeight - 6)), "YES");
	oslDrawString(364 - oslGetStringWidth("NO"), (191 - (font->charHeight - 6)), "NO");
}

void Menu_ControlFileProperties(void) {
	if ((osl_keys->pressed.cross) || (osl_keys->pressed.circle))
		MENU_STATE = MENU_STATE_FILEOPTIONS;
}

void Menu_DisplayFileProperties(void) {
	File *file = Dirbrowse_GetFileIndex(position);

	char path[1024], size[16];
	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);

	oslDrawImageXY(config.dark_theme? properties_dialog_dark : properties_dialog, 131, 32);
	oslIntraFontSetStyle(font, 0.6f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);

	OSL_DrawFillRect((340 - oslGetStringWidth("OK")) - 5, (230 - (font->charHeight - 6)) - 5, oslGetStringWidth("OK") + 10, (font->charHeight - 6) + 10, 
		config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	
	oslDrawString(138, 39, "Properties");
	oslDrawString(340 - oslGetStringWidth("OK"), 230 - (font->charHeight - 6), "OK");

	oslIntraFontSetStyle(font, 0.5f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawStringf(140, 64, "Name: %s", file->name);

	if (!file->isDir) {
		Utils_GetSizeString(size, file->size);
		oslDrawStringf(140, 80, "Size: %s", size);
		oslDrawStringf(140, 96, "Created: %s", FS_GetFileTimestamp(path, 0));
		oslDrawStringf(140, 112, "Accessed: %s", FS_GetFileTimestamp(path, 1));
		oslDrawStringf(140, 128, "Modified: %s", FS_GetFileTimestamp(path, 2));
		oslDrawStringf(140, 144, "Perms: %s", FS_GetFilePermission(path));
	}
	else {
		oslDrawStringf(140, 80, "Created: %s", FS_GetFileTimestamp(path, 0));
		oslDrawStringf(140, 96, "Accessed: %s", FS_GetFileTimestamp(path, 1));
		oslDrawStringf(140, 112, "Modified: %s", FS_GetFileTimestamp(path, 2));
		oslDrawStringf(140, 128, "Perms: %s", FS_GetFilePermission(path));
	}
}

void Menu_ControlFileOptions(void) {
	if (osl_keys->pressed.right)
		row++;
	else if (osl_keys->pressed.left)
		row--;

	if (osl_keys->pressed.down)
		column++;
	else if (osl_keys->pressed.up)
		column--;

	if (!options_more) {
		Utils_SetMax(&row, 0, 1);
		Utils_SetMin(&row, 1, 0);

		Utils_SetMax(&column, 0, 3);
		Utils_SetMin(&column, 3, 0);
	}
	else {
		Utils_SetMax(&column, 0, 2);
		Utils_SetMin(&column, 2, 0);

		if (column == 0) {
			Utils_SetMax(&row, 0, 1);
			Utils_SetMin(&row, 1, 0);
		}
		else if (column == 1) {
			Utils_SetMax(&row, 0, 0);
			Utils_SetMin(&row, 0, 0);
		}
	}
	
	if (osl_keys->pressed.cross) {
		if (row == 0 && column == 0) {
			if (options_more)
				FileOptions_CreateFolder();
			else
				MENU_STATE = MENU_STATE_PROPERTIES;
		}
		else if (row == 1 && column == 0) {
			if (options_more)
				FileOptions_CreateFile();
			else {
				options_more = false;
				row = 0;
				column = 0;
				Dirbrowse_PopulateFiles(true);
				MENU_STATE = MENU_STATE_HOME;
			}
		}
		else if (row == 0 && column == 1) {
			if (options_more)
				FileOptions_Rename();
			else
				HandleCopy();
		}
		else if (row == 1 && column == 1)
			HandleCut();
		else if (row == 0 && column == 2 && !options_more)
			MENU_STATE = MENU_STATE_DELETE;
		else if (row == 1 && column == 2 && !options_more) {
			row = 0;
			column = 0;
			options_more = true;
		}
		else if (column == 3 && !options_more) {
			copy_status = false;
			cut_status = false;
			row = 0;
			column = 0;
			MENU_STATE = MENU_STATE_HOME;
		}
		else if (column == 2 && options_more) {
			options_more = false;
			copy_status = false;
			cut_status = false;
			row = 0;
			column = 0;
			MENU_STATE = MENU_STATE_HOME;
		}
	}

	if (osl_keys->pressed.circle) {
		if (!options_more) {
			copy_status = false;
			cut_status = false;
			row = 0;
			column = 0;
			MENU_STATE = MENU_STATE_HOME;
		}
		else {
			row = 0;
			column = 0;
			options_more = false;
		}
	}

	if (osl_keys->pressed.triangle)
		MENU_STATE = MENU_STATE_HOME;
}

void Menu_DisplayFileOptions(void) {
	oslDrawImageXY(config.dark_theme? options_dialog_dark : options_dialog, 131, 32);
	oslIntraFontSetStyle(font, 0.6f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawString(138, 39, "Actions");
	
	if (row == 0 && column == 0)
		OSL_DrawFillRect(133, 71, 107, 38, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (row == 1 && column == 0)
		OSL_DrawFillRect(241, 71, 107, 38, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (row == 0 && column == 1)
		OSL_DrawFillRect(133, 110, 107, 38, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (row == 1 && column == 1)
		OSL_DrawFillRect(241, 110, 107, 38, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (row == 0 && column == 2 && !options_more)
		OSL_DrawFillRect(133, 148, 107, 38, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (row == 1 && column == 2 && !options_more)
		OSL_DrawFillRect(241, 148, 107, 38, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (column == 3 && !options_more)
		OSL_DrawFillRect((340 - oslGetStringWidth("CANCEL")) - 5, (230 - (font->charHeight - 6)) - 5, oslGetStringWidth("CANCEL") + 10, (font->charHeight - 6) + 10, 
			config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (column == 2 && options_more)
		OSL_DrawFillRect((340 - oslGetStringWidth("CANCEL")) - 5, (230 - (font->charHeight - 6)) - 5, oslGetStringWidth("CANCEL") + 10, (font->charHeight - 6) + 10, 
			config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

	oslDrawString(340 - oslGetStringWidth("CANCEL"), 230 - (font->charHeight - 6), "CANCEL");

	oslIntraFontSetStyle(font, 0.5f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);

	if (!options_more) {
		oslDrawString(143, 82, "Properties");
		oslDrawString(143, 118, copy_status? "Paste" : "Copy");
		oslDrawString(143, 154, "Delete");
		
		oslDrawString(247, 82, "Refresh");
		oslDrawString(247, 118, cut_status? "Paste" : "Move");
		oslDrawString(247, 154, "More...");
	}
	else {
		oslDrawString(143, 82, "New folder");
		oslDrawString(143, 118, "Rename");

		oslDrawString(247, 82, "New file");
	}
}
