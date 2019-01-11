#include <stdbool.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "menu_settings.h"
#include "menu_fileoptions.h"
#include "osl_helper.h"
#include "screenshot.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

#define MENUBAR_X_BOUNDARY  0

static char multi_select_dir_old[512];
static int menubar_selection = 0;
static float menubar_x = -180.0;
u64 total_storage = 0, used_storage = 0;

static void Menu_HandleMultiSelect(void) {
	// multi_select_dir can only hold one dir
	strcpy(multi_select_dir_old, cwd);
	if (strcmp(multi_select_dir_old, multi_select_dir) != 0)
		FileOptions_ResetClipboard();

	char path[512];
	File *file = Dirbrowse_GetFileIndex(position);
	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);
	strcpy(multi_select_dir, cwd);
			
	if (!multi_select[position]) {
		multi_select[position] = true;
		multi_select_indices[position] = multi_select_index; // Store the index in the position
		Utils_AppendArr(multi_select_paths[multi_select_index], path, multi_select_index);
		multi_select_index += 1;
	}
	else {
		multi_select[position] = false;
		strcpy(multi_select_paths[multi_select_indices[position]], "");
		multi_select_indices[position] = -1;
	}

	Utils_SetMax(&multi_select_index, 0, 50);
	Utils_SetMin(&multi_select_index, 50, 0);
}

static void Menu_ControlMenubar(void) {
	char *buf;

	if (osl_keys->pressed.up)
		menubar_selection--;
	else if (osl_keys->pressed.down)
		menubar_selection++;

	Utils_SetMax(&menubar_selection, 0, model_psp_go? 2 : 3);
	Utils_SetMin(&menubar_selection, model_psp_go? 2 : 3, 0);

	if (osl_keys->pressed.value & OSL_KEYMASK_ENTER) {
		switch (menubar_selection) {
			case 0:
				buf = malloc(256);
				memset(root_path, 0, strlen(root_path));
				strcpy(root_path, Utils_IsEF0()? "ef0:/" : "ms0:/");

				if (FS_FileExists("lastdir.txt")) {
					if (R_FAILED(FS_ReadFile("lastdir.txt", buf, 256))) {
						free(buf);
						strcpy(cwd, Utils_IsEF0()? "ef0:/" : "ms0:/");
					}

					char tempPath[256];
					sscanf(buf, "%[^\n]s", tempPath);

					if (FS_DirExists(tempPath)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
						strcpy(cwd, tempPath);
					else
						strcpy(cwd, Utils_IsEF0()? "ef0:/" : "ms0:/");

					free(buf);
				}

				BROWSE_STATE = BROWSE_STATE_SD;
				break;
			case 1:
				memset(root_path, 0, strlen(root_path));
				strcpy(root_path, "flash0:/");
				strcpy(cwd, "flash0:/");
				BROWSE_STATE = BROWSE_STATE_FLASH0;
				break;
			case 2:
				memset(root_path, 0, strlen(root_path));
				strcpy(root_path, "flash1:/");
				strcpy(cwd, "flash1:/");
				BROWSE_STATE = BROWSE_STATE_FLASH1;
				break;
			case 3:
				memset(root_path, 0, strlen(root_path));
				strcpy(root_path, "disc0:/");
				strcpy(cwd, "disc0:/");
				BROWSE_STATE = BROWSE_STATE_UMD;
				break;
		}

		Dirbrowse_PopulateFiles(true);
	}
	else if ((osl_keys->pressed.value & OSL_KEYMASK_CANCEL) || (osl_keys->pressed.select)) {
		menubar_x = -180;
		menubar_selection = 0;
		MENU_STATE = MENU_STATE_HOME;
	}
}

static void Menu_ControlHome(void) {
	if (fileCount > 0) {
		if (osl_keys->pressed.up)
			position--;
		else if (osl_keys->pressed.down)
			position++;

		Utils_SetMax(&position, 0, fileCount - 1);
		Utils_SetMin(&position, fileCount - 1, 0);

		if (osl_keys->pressed.left)
			position = 0;
		else if (osl_keys->pressed.right)
			position = fileCount - 1;

		if (osl_keys->pressed.square)
			Menu_HandleMultiSelect();

		if (osl_keys->pressed.value & OSL_KEYMASK_ENTER)
			Dirbrowse_OpenFile();
		else if ((strcmp(cwd, root_path) != 0) && (osl_keys->pressed.value & OSL_KEYMASK_CANCEL)) {
			Dirbrowse_Navigate(true);
			Dirbrowse_PopulateFiles(true);
		}
	}

	if (osl_keys->pressed.select) {
		menubar_selection = BROWSE_STATE;
		MENU_STATE = MENU_STATE_MENUBAR;
	}
	else if (osl_keys->pressed.start)
		MENU_STATE = MENU_STATE_SETTINGS;
	else if (osl_keys->pressed.triangle)
		MENU_STATE = MENU_STATE_FILEOPTIONS;
}

static void Menu_DisplayMenubar(void) {
	oslDrawImageXY(bg_header, menubar_x, 20);
	OSL_DrawFillRect(menubar_x, 90, 180, 252, config.dark_theme? BLACK_BG : WHITE);
	OSL_DrawFillRect(menubar_x + 180, 20, 1, 252, config.dark_theme? SELECTOR_COLOUR_DARK : RGBA(200, 200, 200, 255));
	OSL_DrawFillRect(menubar_x, 90 + (30 * menubar_selection), 180, 30, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

	oslIntraFontSetStyle(font, 0.6f, config.dark_theme? WHITE : BLACK, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawImageXY(config.dark_theme? icon_sd_dark : icon_sd, menubar_x + 10, 92);
	oslDrawString(menubar_x + 50, 90 + ((30 - (font->charHeight - 6)) / 2), Utils_IsEF0()? "ef0:/" : "ms0:/");

	oslDrawImageXY(config.dark_theme? icon_secure_dark : icon_secure, menubar_x + 10, 122);
	oslDrawString(menubar_x + 50, 90 + ((30 - (font->charHeight - 6)) / 2) + 30, "flash0:/");

	oslDrawImageXY(config.dark_theme? icon_secure_dark : icon_secure, menubar_x + 10, 152);
	oslDrawString(menubar_x + 50, 90 + ((30 - (font->charHeight - 6)) / 2) + 60, "flash1:/");

	if (!model_psp_go) {
		oslDrawImageXY(config.dark_theme? icon_secure_dark : icon_secure, menubar_x + 10, 182);
		oslDrawString(menubar_x + 50, 90 + ((30 - (font->charHeight - 6)) / 2) + 90, "disc0:/");
	}
}

void Menu_Main(void) {
	Dirbrowse_PopulateFiles(false);
	memset(multi_select, 0, sizeof(multi_select)); // Reset all multi selected items

	total_storage = Utils_GetTotalStorage();
	used_storage = Utils_GetUsedStorage();

	while (!osl_quit) {
		OSL_StartDrawing();
		oslClearScreen(config.dark_theme? BLACK_BG : WHITE);
		OSL_DrawFillRect(0, 0, 480, 20, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
		OSL_DrawFillRect(0, 20, 480, 42, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
		oslDrawImageXY(icon_nav_drawer, 5, 25);

		StatusBar_DisplayTime();
		Dirbrowse_DisplayFiles();

		oslReadKeys();
		if (MENU_STATE == MENU_STATE_HOME)
			Menu_ControlHome();
		else if (MENU_STATE == MENU_STATE_FILEOPTIONS) {
			Menu_DisplayFileOptions();
			Menu_ControlFileOptions();
		}
		else if (MENU_STATE == MENU_STATE_PROPERTIES) {
			Menu_DisplayFileProperties();
			Menu_ControlFileProperties();
		}
		else if (MENU_STATE == MENU_STATE_DELETE) {
			Menu_DisplayDeleteDialog();
			Menu_ControlDeleteDialog();
		}
		else if (MENU_STATE == MENU_STATE_MENUBAR) {
			menubar_x += 10.0;

			if (menubar_x > -1)
				menubar_x = MENUBAR_X_BOUNDARY;
			Menu_ControlMenubar();
			Menu_DisplayMenubar();
		}
		else if (MENU_STATE == MENU_STATE_SETTINGS)
			Menu_DisplaySettings();

		if (((osl_keys->held.L) && (osl_keys->pressed.R)) || ((osl_keys->held.R) && (osl_keys->pressed.L)))
			Screenshot_Capture();

		else if (((osl_keys->held.start) && (osl_keys->pressed.select)) || ((osl_keys->held.select) && (osl_keys->pressed.start)))
			longjmp(exitJmp, 1);

		Utils_HandleUSB();
		OSL_EndDrawing();
	}
}
