#include <stdbool.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "menu_settings.h"
#include "menu_fileoptions.h"
#include "osl_helper.h"
#include "screenshot.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

static char multi_select_dir_old[512];

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

		if (osl_keys->pressed.cross)
			Dirbrowse_OpenFile();
		else if ((strcmp(cwd, ROOT_PATH) != 0) && (osl_keys->pressed.circle)) {
			Dirbrowse_Navigate(true);
			Dirbrowse_PopulateFiles(true);
		}
	}

	if (osl_keys->pressed.start)
		MENU_STATE = MENU_STATE_SETTINGS;
	else if (osl_keys->pressed.triangle)
		MENU_STATE = MENU_STATE_FILEOPTIONS;
}

void Menu_Main(void) {
	Dirbrowse_PopulateFiles(false);
	memset(multi_select, 0, sizeof(multi_select)); // Reset all multi selected items

	while (!osl_quit) {
		OSL_StartDrawing();
		oslClearScreen(config.dark_theme? BLACK_BG : WHITE);
		OSL_DawFillRect(0, 0, 480, 20, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
		OSL_DawFillRect(0, 20, 480, 42, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
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
