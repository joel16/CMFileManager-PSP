#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#include "archive.h"
#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "glib2d_helper.h"
#include "fs.h"
#include "menu_error.h"
#include "progress_bar.h"
#include "status_bar.h"
#include "textures.h"
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

static int Archive_ExtractZIP(const char *src) {
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

	sceIoChdir(initial_cwd);
	return res;
}

int Archive_ExtractFile(const char *path) {
	int dialog_selection = 0;
	int text_width1 = intraFontMeasureText(font, "This may take a few minutes.");
	int text_width2 = intraFontMeasureText(font, "Do you want to continue?");

	while (1) {
		g2dClear(config.dark_theme? BLACK_BG : WHITE);
		G2D_DrawRect(0, 0, 480, 20, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
		G2D_DrawRect(0, 20, 480, 42, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
		G2D_DrawImage(icon_nav_drawer, 5, 25);

		StatusBar_DisplayTime();
		Dirbrowse_DisplayFiles();

		G2D_DrawRect(0, 20, 480, 252, G2D_RGBA(0, 0, 0, config.dark_theme? 50: 80));

		G2D_DrawImage(config.dark_theme? dialog_dark : dialog, ((480 - dialog->w) / 2), ((272 - dialog->h) / 2));

		intraFontSetStyle(font, 0.7f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
		intraFontPrint(font, ((480 - dialog->w) / 2) + 10, ((272 - dialog->h) / 2) + 20, "Extract file");

		intraFontSetStyle(font, 0.7f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
		intraFontPrint(font, ((480 - (text_width1)) / 2), ((272 - dialog->h) / 2) + 50, "This may take a few minutes.");
		intraFontPrint(font, ((480 - (text_width2)) / 2), ((272 - dialog->h) / 2) + 66, "Do you wish to continue?");

		intraFontSetStyle(font, 0.7f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);

		if (dialog_selection == 0)
			G2D_DrawRect((364 - intraFontMeasureText(font, "NO")) - 5, (180 - (font->texYSize - 20)) - 5, intraFontMeasureText(font, "NO") + 10, (font->texYSize - 10) + 10, 
				config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
		else if (dialog_selection == 1)
			G2D_DrawRect((409 - (intraFontMeasureText(font, "YES"))) - 5, (180 - (font->texYSize - 20)) - 5, intraFontMeasureText(font, "YES") + 10, (font->texYSize - 10) + 10, 
				config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

		intraFontPrint(font, 409 - (intraFontMeasureText(font, "YES")), (182 - (font->texYSize - 30)), "YES");
		intraFontPrint(font, 364 - intraFontMeasureText(font, "NO"), (182 - (font->texYSize - 30)), "NO");

		g2dFlip(G2D_VSYNC);

		Utils_ReadControls();

		if (Utils_IsButtonPressed(PSP_CTRL_RIGHT))
			dialog_selection++;
		else if (Utils_IsButtonPressed(PSP_CTRL_LEFT))
			dialog_selection--;

		Utils_SetMax(&dialog_selection, 0, 1);
		Utils_SetMin(&dialog_selection, 1, 0);

		if (Utils_IsButtonPressed(PSP_CTRL_CANCEL))
			break;

		if (Utils_IsButtonPressed(PSP_CTRL_ENTER)) {
			if (dialog_selection == 1) {
				return Archive_ExtractZIP(path);
			}
			else
				break;
		}
	}

	return -1;
}
