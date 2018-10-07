#include <stdbool.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
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
static bool copy_status = false, cut_status = false;

void FileOptions_ResetClipboard(void) {
	multi_select_index = 0;
	memset(multi_select, 0, sizeof(multi_select));
	memset(multi_select_indices, 0, sizeof(multi_select_indices));
	memset(multi_select_dir, 0, sizeof(multi_select_dir));
	memset(multi_select_paths, 0, sizeof(multi_select_paths));
}

static int FileOptions_CreateFolder(void) {
	char *buf = (char *)malloc(256);
	Utils_DisplayKeyboard("Enter name", "", buf);

	if (strncmp(buf, "", 1) == 0)
		return -1;

	char path[512];
	strcpy(path, cwd);
	strcat(path, buf);
	free(buf);

	FS_RecursiveMakeDir(path);
	Dirbrowse_PopulateFiles(true);
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

	char *buf = (char *)malloc(256);

	strcpy(oldPath, cwd);
	strcpy(newPath, cwd);
	strcat(oldPath, file->name);
	
	Utils_DisplayKeyboard("Enter name", "", buf);
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
	return 0;
}

static int FileOptions_RmdirRecursive(char *path)
{
	int dir = 0, i = 0;
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
				if (strcmp(entries[i].d_name, ".") == 0 || strcmp(entries[i].d_name, "..") == 0)
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
			char *buffer = (char *)malloc(size);

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
			char *buffer = (char *)malloc(size);

			// Combine Path
			strcpy(buffer, path);
			strcpy(buffer + strlen(buffer), node->name);

			// Delete File
			sceIoRemove(buffer);
			free(buffer);
		}
	}

	// Free temporary List
	Dirbrowse_RecursiveFree(filelist);
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
	int i = 0;

	if ((multi_select_index > 0) && (strlen(multi_select_dir) != 0)) {
		for (i = 0; i < multi_select_index; i++) {
			if (strlen(multi_select_paths[i]) != 0) {
				if (strncmp(multi_select_paths[i], "..", 2) != 0) {
					if (FS_DirExists(multi_select_paths[i]))
						FileOptions_RmdirRecursive(multi_select_paths[i]);
					else if (FS_FileExists(multi_select_paths[i]))
						sceIoRemove(multi_select_paths[i]);
				}
			}
		}

		FileOptions_ResetClipboard();
	}
	else if (FileOptions_DeleteFile() != 0)
		return;

	Dirbrowse_PopulateFiles(true);
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

static int FileOptions_CopyFile(char *src, char *dst, bool display_anim)
{
	int chunksize = (512 * 1024); // Chunk size
	char *buffer = (char *)malloc(chunksize); // Reading buffer

	int totalwrite = 0, totalread = 0, ret = 0, input_file = 0, output_file = 0;

	if (R_SUCCEEDED(input_file = sceIoOpen(src, PSP_O_RDONLY, 0777))) {
		sceIoRemove(dst); // Delete Output File (if existing)

		if (R_SUCCEEDED(output_file = sceIoOpen(dst, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777)) >= 0) {
			int read = 0;

			// Copy Loop (512KB at a time)
			while((read = sceIoRead(input_file, buffer, chunksize)) > 0) {
				// Accumulate Read Data
				totalread += read;
				totalwrite += sceIoWrite(output_file, buffer, read);
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

				// Calculate Buffer Size
				int insize = strlen(src) + strlen(entries[i].d_name) + 2;
				int outsize = strlen(dst) + strlen(entries[i].d_name) + 2;

				// Allocate Buffer
				char *inbuffer = (char *)malloc(insize);
				char *outbuffer = (char *)malloc(outsize);

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
	char *copytarget = (char *)malloc(requiredlength); // Allocate target path buffer

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

		if ((R_SUCCEEDED(ret)) && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
			FileOptions_RmdirRecursive(copysource); // Delete dir
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
		else if (FileOptions_Paste() != 0)
			return;

		copy_status = false;
		Dirbrowse_PopulateFiles(true);
		MENU_STATE = MENU_STATE_HOME;
	}
}

static void HandleCut(void) {
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
						sceIoRename(multi_select_paths[i], dest);
					else if (FS_FileExists(multi_select_paths[i]))
						sceIoRename(multi_select_paths[i], dest);
				}
			}

			FileOptions_ResetClipboard();
		}
		else {
			snprintf(dest, 512, "%s%s", cwd, Utils_Basename(copysource));

			if (FS_DirExists(copysource))
				sceIoRename(copysource, dest);
			else if (FS_FileExists(copysource))
				sceIoRename(copysource, dest);
		}

		cut_status = false;
		copymode = NOTHING_TO_COPY;
		Dirbrowse_PopulateFiles(true);
		MENU_STATE = MENU_STATE_HOME;
	}
}

void Menu_DisplayDeleteDialog(void) {
	int text_width = oslGetStringWidth("Do you want to continue?");

	oslDrawImageXY(config_dark_theme? dialog_dark : dialog, ((480 - oslGetImageWidth(dialog)) / 2), ((272 - oslGetImageHeight(dialog)) / 2));

	oslIntraFontSetStyle(font, 0.6f, config_dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawString(((480 - oslGetImageWidth(dialog)) / 2) + 8, ((272 - oslGetImageHeight(dialog)) / 2) + 6, "Confirm deletion");

	oslIntraFontSetStyle(font, 0.6f, config_dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawString(((480 - (text_width)) / 2), ((272 - oslGetImageHeight(dialog)) / 2) + 40, "Do you wish to continue?");

	if (delete_dialog_selection == 0)
		oslDrawFillRect((364 - oslGetStringWidth("NO")) - 5, (191 - (font->charHeight - 6)) - 5, 
			((364 - oslGetStringWidth("NO")) - 5) + oslGetStringWidth("NO") + 10, ((191 - (font->charHeight - 6)) - 5) + (font->charHeight - 6) + 10, 
			config_dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (delete_dialog_selection == 1)
		oslDrawFillRect((409 - (oslGetStringWidth("YES"))) - 5, (191 - (font->charHeight - 6)) - 5, 
			((409 - (oslGetStringWidth("YES"))) - 5) + oslGetStringWidth("YES") + 10, ((191 - (font->charHeight - 6)) - 5) + (font->charHeight - 6) + 10, 
			config_dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

	// 419
	oslIntraFontSetStyle(font, 0.6f, config_dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
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

	oslDrawImageXY(config_dark_theme? properties_dialog_dark : properties_dialog, 131, 32);
	oslIntraFontSetStyle(font, 0.6f, config_dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawString(138, 39, "Properties");
	oslDrawString(345 - oslGetStringWidth("OK"), 230 - (font->charHeight - 6), "OK");

	oslIntraFontSetStyle(font, 0.5f, config_dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawStringf(140, 64, "Name: %s", file->name);
	oslDrawStringf(140, 80, "Parent: %s", cwd);

	if (!file->isDir) {
		Utils_GetSizeString(size, file->size);
		oslDrawStringf(140, 96, "Size: %s", size);
		oslDrawStringf(140, 112, "Creation time: %s", FS_GetFileTimestamp(path, 0));
		oslDrawStringf(140, 128, "Access time: %s", FS_GetFileTimestamp(path, 1));
		oslDrawStringf(140, 144, "Modification time: %s", FS_GetFileTimestamp(path, 2));
	}
	else {
		oslDrawStringf(140, 96, "Creation time: %s", FS_GetFileTimestamp(path, 0));
		oslDrawStringf(140, 112, "Access time: %s", FS_GetFileTimestamp(path, 1));
		oslDrawStringf(140, 128, "Modification time: %s", FS_GetFileTimestamp(path, 2));
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

	Utils_SetMax(&row, 0, 1);
	Utils_SetMin(&row, 1, 0);

	Utils_SetMax(&column, 0, 2);
	Utils_SetMin(&column, 2, 0);
	
	if (osl_keys->pressed.cross) {
		if (row == 0 && column == 0)
			MENU_STATE = MENU_STATE_PROPERTIES;
		else if (row == 1 && column == 0)
			FileOptions_CreateFolder();
		else if (row == 0 && column == 1)
			FileOptions_Rename();
		else if (row == 1 && column == 1)
			HandleCopy();
		else if (row == 0 && column == 2)
			HandleCut();
		else if (row == 1 && column == 2)
			MENU_STATE = MENU_STATE_DELETE;
	}

	if (osl_keys->pressed.circle) {
		copy_status = false;
		cut_status = false;
		row = 0;
		column = 0;
		MENU_STATE = MENU_STATE_HOME;
	}

	if (osl_keys->pressed.triangle)
		MENU_STATE = MENU_STATE_HOME;
}

void Menu_DisplayFileOptions(void) {
	oslDrawImageXY(config_dark_theme? options_dialog_dark : options_dialog, 131, 32);
	oslIntraFontSetStyle(font, 0.6f, config_dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawString(138, 39, "Actions");

	oslDrawString(345 - oslGetStringWidth("CANCEL"), 230 - (font->charHeight - 6), "CANCEL");
	
	if (row == 0 && column == 0)
		oslDrawFillRect(133, 71, 133 + 107, 71 + 38, config_dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (row == 1 && column == 0)
		oslDrawFillRect(241, 71, 241 + 107, 71 + 38, config_dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (row == 0 && column == 1)
		oslDrawFillRect(133, 110, 133 + 107, 110 + 38, config_dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (row == 1 && column == 1)
		oslDrawFillRect(241, 110, 241 + 107, 110 + 38, config_dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (row == 0 && column == 2)
		oslDrawFillRect(133, 148, 133 + 107, 148 + 38, config_dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (row == 1 && column == 2)
		oslDrawFillRect(241, 148, 241 + 107, 148 + 38, config_dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

	oslIntraFontSetStyle(font, 0.5f, config_dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);

	oslDrawString(143, 82, "Properties");
	oslDrawString(143, 118, "Rename");
	oslDrawString(143, 154, cut_status? "Paste" : "Move");
		
	oslDrawString(247, 82, "New folder");
	oslDrawString(247, 118, copy_status? "Paste" : "Copy");
	oslDrawString(247, 154, "Delete");
}
