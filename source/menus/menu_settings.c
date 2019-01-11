#include <stdbool.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "osl_helper.h"
#include "screenshot.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

static bool displayAbout, displaySupport;

static void Menu_DisplaySortSettings(void) {
	int selection = 0, max_items = 3, i = 0;

	const char *main_menu_items[] = {
		"By name (ascending)",
		"By name (descending)",
		"By size (largest first)",
		"By size (smallest first)"
	};

	int width = oslGetImageWidth(icon_radio_on);

	while (!osl_quit) {
		OSL_StartDrawing();
		oslClearScreen(config.dark_theme? BLACK_BG : WHITE);
		OSL_DrawFillRect(0, 0, 480, 20, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
		OSL_DrawFillRect(0, 20, 480, 42, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);

		StatusBar_DisplayTime();

		oslDrawImageXY(icon_back, 5, 25);
		oslIntraFontSetStyle(font, 0.6f, WHITE, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
		oslDrawString(40, 20 + ((40 - (font->charHeight - 6)) / 2), "Sorting Options");

		int printed = 0; // Print counter

		for (i = 0; i < max_items + 1; i++) {
			if (printed == FILES_PER_PAGE)
				break;

			if (selection < FILES_PER_PAGE || i > (selection - FILES_PER_PAGE)) {
				if (i == selection)
					OSL_DrawFillRect(0, 62 + (42 * printed), 480, 42, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
				
				oslIntraFontSetStyle(font, 0.6f, config.dark_theme? WHITE : BLACK, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
				oslDrawString(20, 62 + ((42 - (font->charHeight - 6)) / 2) + (42 * printed), main_menu_items[i]);

				printed++;
			}
		}

		config.sort == 0? oslDrawImageXY(config.dark_theme? icon_radio_dark_on : icon_radio_on, (460 - width), 69) : 
			oslDrawImageXY(config.dark_theme? icon_radio_dark_off : icon_radio_off, (460 - width), 69);
		
		config.sort == 1? oslDrawImageXY(config.dark_theme? icon_radio_dark_on : icon_radio_on, (460 - width), 111) : 
			oslDrawImageXY(config.dark_theme? icon_radio_dark_off : icon_radio_off, (460 - width), 111);

		config.sort == 2? oslDrawImageXY(config.dark_theme? icon_radio_dark_on : icon_radio_on, (460 - width), 153) : 
			oslDrawImageXY(config.dark_theme? icon_radio_dark_off : icon_radio_off, (460 - width), 153);
		
		config.sort == 3? oslDrawImageXY(config.dark_theme? icon_radio_dark_on : icon_radio_on, (460 - width), 195) : 
			oslDrawImageXY(config.dark_theme? icon_radio_dark_off : icon_radio_off, (460 - width), 195);
		
		OSL_EndDrawing();

		oslReadKeys();

		if (osl_keys->pressed.value & OSL_KEYMASK_CANCEL)
			break;
			
		if (osl_keys->pressed.down)
			selection++;
		else if (osl_keys->pressed.up)
			selection--;

		if (((osl_keys->held.L) && (osl_keys->pressed.R)) || ((osl_keys->held.R) && (osl_keys->pressed.L)))
			Screenshot_Capture();

		Utils_SetMax(&selection, 0, max_items);
		Utils_SetMin(&selection, max_items, 0);

		if (osl_keys->pressed.value & OSL_KEYMASK_ENTER) {
			switch (selection) {
				case 0:
					config.sort = 0;
					break;
				case 1:
					config.sort = 1;
					break;
				case 2:
					config.sort = 2;
					break;
				case 3:
					config.sort = 3;
					break;
			}

			Config_Save(config);
		}
	}
	Dirbrowse_PopulateFiles(true);
}

static void Menu_ControlAboutDialog(void) {
	if ((osl_keys->pressed.value & OSL_KEYMASK_ENTER) || (osl_keys->pressed.value & OSL_KEYMASK_CANCEL))
		displayAbout = false;
}

static void Menu_DisplayAboutDialog(void) {
	int text_width = oslGetStringWidth("CM File Manager PSP vX.X.X");
	int author_width = oslGetStringWidth("Author: Joel16");

	oslDrawImageXY(config.dark_theme? dialog_dark : dialog, ((480 - oslGetImageWidth(dialog)) / 2), ((272 - oslGetImageHeight(dialog)) / 2));

	oslIntraFontSetStyle(font, 0.6f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawString(((480 - oslGetImageWidth(dialog)) / 2) + 6, ((272 - oslGetImageHeight(dialog)) / 2) + 6, "About");

	oslIntraFontSetStyle(font, 0.6f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawStringf(((480 - (text_width)) / 2), ((272 - oslGetImageHeight(dialog)) / 2) + 40, "CM File Manager PSP v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
	oslDrawString(((480 - (author_width)) / 2), ((272 - oslGetImageHeight(dialog)) / 2) + 40 + 16, "Author: Joel16");

	OSL_DrawFillRect((409 - (oslGetStringWidth("OK"))) - 5, (191 - (font->charHeight - 6)) - 5, oslGetStringWidth("OK") + 10, (font->charHeight - 6) + 10, 
		config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

	oslIntraFontSetStyle(font, 0.6f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawString(409 - (oslGetStringWidth("OK")), (191 - (font->charHeight - 6)), "OK");
}

static void Menu_ControlSupportDialog(void) {
	if ((osl_keys->pressed.value & OSL_KEYMASK_ENTER) || (osl_keys->pressed.value & OSL_KEYMASK_CANCEL))
		displaySupport = false;
}

static void Menu_DisplaySupportDialog(void) {
	oslDrawImageXY(config.dark_theme? properties_dialog_dark : properties_dialog, 131, 32);
	oslIntraFontSetStyle(font, 0.6f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);

	OSL_DrawFillRect((340 - oslGetStringWidth("OK")) - 5, (230 - (font->charHeight - 6)) - 5, oslGetStringWidth("OK") + 10, (font->charHeight - 6) + 10, 
		config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

	oslDrawString(138, 39, "Support");
	oslDrawString(340 - oslGetStringWidth("OK"), 230 - (font->charHeight - 6), "OK");

	oslIntraFontSetStyle(font, 0.5f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawString(140, 64, "DPAD Up: Cursor up");
	oslDrawString(140, 80, "DPAD Down: Cursor down");
	oslDrawString(140, 96, "DPAD Left: Start of listing");
	oslDrawString(140, 112, "DPAD Right: End of listing");
	oslDrawString(140, 128, "Triangle: File options");
	oslDrawString(140, 144, "Start: Open settings");
	oslDrawString(140, 160, "Select: Open menu bar");
	oslDrawString(140, 176, "L + R: Screenshot");
}

void Menu_DisplaySettings(void) {
	int selection = 0, max_items = 4, i = 0;

	const char *main_menu_items[] = {
		"Sorting options",
		"Dark theme",
		"Auto USB mount",
		"Support",
		"About",
	};

	displayAbout = false;

	int width = oslGetImageWidth(icon_toggle_on);
	int height = oslGetImageHeight(icon_toggle_on);

	while (!osl_quit) {
		OSL_StartDrawing();
		oslClearScreen(config.dark_theme? BLACK_BG : WHITE);
		OSL_DrawFillRect(0, 0, 480, 20, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
		OSL_DrawFillRect(0, 20, 480, 42, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);

		StatusBar_DisplayTime();

		oslDrawImageXY(icon_back, 5, 25);
		oslIntraFontSetStyle(font, 0.6f, WHITE, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
		oslDrawString(40, 20 + ((40 - (font->charHeight - 6)) / 2), "Settings");

		int printed = 0; // Print counter

		for (i = 0; i < max_items + 1; i++) {
			if (printed == FILES_PER_PAGE)
				break;

			if (selection < FILES_PER_PAGE || i > (selection - FILES_PER_PAGE)) {
				if (i == selection)
					OSL_DrawFillRect(0, 62 + (42 * printed), 480, 42, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

				oslIntraFontSetStyle(font, 0.6f, config.dark_theme? WHITE : BLACK, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
				oslDrawString(20, 62 + ((42 - (font->charHeight - 6)) / 2) + (42 * printed), main_menu_items[i]);

				printed++;
			}
		}

		if (config.dark_theme)
			oslDrawImageXY(config.dark_theme? icon_toggle_dark_on : icon_toggle_off, 455 - width, 104 + ((42 - height) / 2));
		else
			oslDrawImageXY(config.dark_theme? icon_toggle_on : icon_toggle_off, 455 - width,  104 + ((42 - height) / 2));

		if (config.dark_theme)
			oslDrawImageXY(config.auto_usb_mount? icon_toggle_dark_on : icon_toggle_off, 455 - width, 146 + ((42 - height) / 2));
		else
			oslDrawImageXY(config.auto_usb_mount? icon_toggle_on : icon_toggle_off, 455 - width,  146 + ((42 - height) / 2));

		oslReadKeys();

		if (((osl_keys->held.L) && (osl_keys->pressed.R)) || ((osl_keys->held.R) && (osl_keys->pressed.L)))
			Screenshot_Capture();

		if (displayAbout) {
			Menu_DisplayAboutDialog();
			Menu_ControlAboutDialog();
		}
		else if (displaySupport) {
			Menu_DisplaySupportDialog();
			Menu_ControlSupportDialog();
		}
		else {
			if ((osl_keys->pressed.value & OSL_KEYMASK_CANCEL) || (osl_keys->pressed.start))
				break;
			
			if (osl_keys->pressed.down)
				selection++;
			else if (osl_keys->pressed.up)
				selection--;

			Utils_SetMax(&selection, 0, max_items);
			Utils_SetMin(&selection, max_items, 0);

			if (osl_keys->pressed.value & OSL_KEYMASK_ENTER) {
				switch (selection) {
					case 0:
						Menu_DisplaySortSettings();
						break;
					case 1:
						config.dark_theme = !config.dark_theme;
						Config_Save(config);
						break;
					case 2:
						config.auto_usb_mount = !config.auto_usb_mount;
						Config_Save(config);
						break;
					case 3:
						displaySupport = true;
						break;
					case 4:
						displayAbout = true;
						break;
				}
			}
		}

		OSL_EndDrawing();
	}

	MENU_STATE = MENU_STATE_HOME;
}
