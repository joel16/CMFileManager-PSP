#include <archive.h>
#include <archive_entry.h>
#include <FLAC/format.h>
#include <opus.h>
#include <xmp.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "glib2d_helper.h"
#include "menu_ftp.h"
#include "screenshot.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

#define MAX_ITEMS_PAGE 5

static bool display_about, display_support;

static void Menu_DisplaySortSettings(void) {
	int selection = 0, max_items = 3, i = 0;

	const char *main_menu_items[] = {
		"By name (ascending)",
		"By name (descending)",
		"By size (largest first)",
		"By size (smallest first)"
	};

	int width = icon_radio_on->w;

	while (1) {
		g2dClear(config.dark_theme? BLACK_BG : WHITE);
		G2D_DrawRect(0, 0, 480, 20, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
		G2D_DrawRect(0, 20, 480, 42, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);

		StatusBar_DisplayTime();

		G2D_DrawImage(icon_back, 5, 25);
		intraFontSetStyle(font, 0.7f, WHITE, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
		intraFontPrint(font, 40, 23 + ((40 - (font->texYSize - 30)) / 2), "Sorting Options");

		int printed = 0; // Print counter

		for (i = 0; i < max_items + 1; i++) {
			if (printed == MAX_ITEMS_PAGE)
				break;

			if (selection < MAX_ITEMS_PAGE || i > (selection - MAX_ITEMS_PAGE)) {
				if (i == selection)
					G2D_DrawRect(0, 62 + (42 * printed), 480, 42, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
				
				intraFontSetStyle(font, 0.7f, config.dark_theme? WHITE : BLACK, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
				intraFontPrint(font, 20, 64 + ((42 - (font->texYSize - 30)) / 2) + (42 * printed), main_menu_items[i]);

				printed++;
			}
		}
		
		G2D_DrawImage(config.sort == 0? (config.dark_theme? icon_radio_dark_on : icon_radio_on) : (config.dark_theme? icon_radio_dark_off : icon_radio_off), (460 - width), 69);
		G2D_DrawImage(config.sort == 1? (config.dark_theme? icon_radio_dark_on : icon_radio_on) : (config.dark_theme? icon_radio_dark_off : icon_radio_off), (460 - width), 111);
		G2D_DrawImage(config.sort == 2? (config.dark_theme? icon_radio_dark_on : icon_radio_on) : (config.dark_theme? icon_radio_dark_off : icon_radio_off), (460 - width), 153);
		G2D_DrawImage(config.sort == 3? (config.dark_theme? icon_radio_dark_on : icon_radio_on) : (config.dark_theme? icon_radio_dark_off : icon_radio_off), (460 - width), 195);
		
		g2dFlip(G2D_VSYNC);

		int ctrl = Utils_ReadControls();

		if (Utils_IsButtonPressed(PSP_CTRL_CANCEL))
			break;
			
		if (ctrl & PSP_CTRL_DOWN)
			selection++;
		else if (ctrl & PSP_CTRL_UP)
			selection--;

		if (Utils_IsButtonPressed(PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER))
			Screenshot_Capture();

		Utils_SetMax(&selection, 0, max_items);
		Utils_SetMin(&selection, max_items, 0);

		if (Utils_IsButtonPressed(PSP_CTRL_ENTER)) {
			config.sort = selection;
			Config_Save(config);
		}
	}
	Dirbrowse_PopulateFiles(true);
}

static void Menu_ControlAboutDialog(void) {
	if ((Utils_IsButtonPressed(PSP_CTRL_ENTER)) || (Utils_IsButtonPressed(PSP_CTRL_CANCEL)))
		display_about = false;
}

static void Menu_DisplayAboutDialog(void) {
	G2D_DrawRect(0, 20, 480, 252, G2D_RGBA(0, 0, 0, config.dark_theme? 50: 80));

	G2D_DrawImage(config.dark_theme? properties_dialog_dark : properties_dialog, 131, 32);
	intraFontSetStyle(font, 0.7f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);

	G2D_DrawRect((340 - intraFontMeasureText(font, "OK")) - 5, (230 - (font->texYSize - 6)) - 5, intraFontMeasureText(font, "OK") + 10, (font->texYSize - 6) + 10, 
		config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

	intraFontPrint(font, 138, 50, "About");
	intraFontPrint(font, 340 - intraFontMeasureText(font, "OK"), 230 - (font->texYSize - 20), "OK");

	intraFontSetStyle(font, 0.7f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
	intraFontPrintf(font, 140, 74, "CM File Manager PSP v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
	intraFontPrint(font, 140, 90, "Author: Joel16");
	intraFontPrint(font, 140, 122, "Libraries:");
	intraFontPrintf(font, 140, 138, "libarchive v%d.%d.%d", ARCHIVE_VERSION_NUMBER / 1000000, (ARCHIVE_VERSION_NUMBER / 1000) % 1000, ARCHIVE_VERSION_NUMBER % 1000);
	intraFontPrintf(font, 140, 154, "libflac v%s", FLAC__VERSION_STRING);
	intraFontPrintf(font, 140, 170, "libxmp v%s", XMP_VERSION);
	intraFontPrint(font, 140, 186, opus_get_version_string());
}

static void Menu_ControlSupportDialog(void) {
	if ((Utils_IsButtonPressed(PSP_CTRL_ENTER)) || (Utils_IsButtonPressed(PSP_CTRL_CANCEL)))
		display_support = false;
}

static void Menu_DisplaySupportDialog(void) {
	G2D_DrawRect(0, 20, 480, 252, G2D_RGBA(0, 0, 0, config.dark_theme? 50: 80));

	G2D_DrawImage(config.dark_theme? properties_dialog_dark : properties_dialog, 131, 32);
	intraFontSetStyle(font, 0.7f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);

	G2D_DrawRect((340 - intraFontMeasureText(font, "OK")) - 5, (230 - (font->texYSize - 6)) - 5, intraFontMeasureText(font, "OK") + 10, (font->texYSize - 6) + 10, 
		config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

	intraFontPrint(font, 138, 50, "Support");
	intraFontPrint(font, 340 - intraFontMeasureText(font, "OK"), 230 - (font->texYSize - 20), "OK");

	intraFontSetStyle(font, 0.7f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
	intraFontPrint(font, 140, 74, "DPAD Up: Cursor up");
	intraFontPrint(font, 140, 90, "DPAD Down: Cursor down");
	intraFontPrint(font, 140, 106, "DPAD Left: Start of listing");
	intraFontPrint(font, 140, 122, "DPAD Right: End of listing");
	intraFontPrint(font, 140, 138, "Triangle: File options");
	intraFontPrint(font, 140, 154, "Start: Open settings");
	intraFontPrint(font, 140, 170, "Select: Open menu bar");
	intraFontPrint(font, 140, 186, "L + R: Screenshot");
}

void Menu_DisplaySettings(void) {
	int selection = 0, max_items = 7, i = 0;

	const char *main_menu_items[] = {
		"FTP connection",
		"Sorting options",
		"Dark theme",
		"Auto USB mount",
		"Large icons",
		"Developer options",
		"Support",
		"About"
	};

	display_about = false;

	int width = icon_toggle_on->w;

	while (1) {
		g2dClear(config.dark_theme? BLACK_BG : WHITE);
		G2D_DrawRect(0, 0, 480, 20, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
		G2D_DrawRect(0, 20, 480, 42, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);

		StatusBar_DisplayTime();

		G2D_DrawImage(icon_back, 5, 25);
		intraFontSetStyle(font, 0.7f, WHITE, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
		intraFontPrint(font, 40, 23 + ((40 - (font->texYSize - 30)) / 2), "Settings");

		int printed = 0; // Print counter

		for (i = 0; i < max_items + 1; i++) {
			if (printed == MAX_ITEMS_PAGE)
				break;

			if (selection < MAX_ITEMS_PAGE || i > (selection - MAX_ITEMS_PAGE)) {
				if (i == selection)
					G2D_DrawRect(0, 62 + (42 * printed), 480, 42, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

				intraFontSetStyle(font, 0.7f, config.dark_theme? WHITE : BLACK, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
				intraFontPrint(font, 20, 64 + ((42 - (font->texYSize - 30)) / 2) + (42 * printed), main_menu_items[i]);

				if (i == 2)
					G2D_DrawImage(config.dark_theme? (config.dark_theme? icon_toggle_dark_on : icon_toggle_on) : icon_toggle_off, 455 - width, 66 + (42 * printed));
				else if (i == 3)
					G2D_DrawImage(config.auto_usb_mount? (config.dark_theme? icon_toggle_dark_on : icon_toggle_on) : icon_toggle_off, 455 - width, 66 + (42 * printed));
				else if (i == 4)
					G2D_DrawImage(config.large_icons? (config.dark_theme? icon_toggle_dark_on : icon_toggle_on) : icon_toggle_off, 455 - width, 66 + (42 * printed));
				else if (i == 5)
					G2D_DrawImage(config.dev_options? (config.dark_theme? icon_toggle_dark_on : icon_toggle_on) : icon_toggle_off, 455 - width, 66 + (42 * printed));

				printed++;
			}
		}

		if (display_about)
			Menu_DisplayAboutDialog();
		else if (display_support)
			Menu_DisplaySupportDialog();

		g2dFlip(G2D_VSYNC);

		int ctrl = Utils_ReadControls();

		if (Utils_IsButtonPressed(PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER))
			Screenshot_Capture();

		if (display_about)
			Menu_ControlAboutDialog();
		else if (display_support)
			Menu_ControlSupportDialog();
		else {
			if ((Utils_IsButtonPressed(PSP_CTRL_CANCEL)) || (Utils_IsButtonPressed(PSP_CTRL_START)))
				break;
			
			if (ctrl & PSP_CTRL_DOWN)
				selection++;
			else if (ctrl & PSP_CTRL_UP)
				selection--;

			Utils_SetMax(&selection, 0, max_items);
			Utils_SetMin(&selection, max_items, 0);

			if (Utils_IsButtonPressed(PSP_CTRL_ENTER)) {
				switch (selection) {
					case 0:
						Menu_DisplayFTP();
						Dirbrowse_PopulateFiles(true);
						break;
					case 1:
						Menu_DisplaySortSettings();
						break;
					case 2:
						config.dark_theme = !config.dark_theme;
						Config_Save(config);
						break;
					case 3:
						config.auto_usb_mount = !config.auto_usb_mount;
						Config_Save(config);
						break;
					case 4:
						config.large_icons = !config.large_icons;
						Config_Save(config);
						break;
					case 5:
						config.dev_options = !config.dev_options;
						Config_Save(config);
						break;
					case 6:
						display_support = true;
						break;
					case 7:
						display_about = true;
						break;
				}
			}
		}
	}

	MENU_STATE = MENU_STATE_HOME;
}
