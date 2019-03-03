#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "glib2d_helper.h"
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

	int width = icon_radio_on->w;

	while (1) {
		g2dClear(config.dark_theme? BLACK_BG : WHITE);
		G2D_DrawRect(0, 0, 480, 20, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
		G2D_DrawRect(0, 20, 480, 42, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);

		StatusBar_DisplayTime();

		G2D_DrawImage(icon_back, 5, 25);
		intraFontSetStyle(font, 0.7f, WHITE, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
		intraFontPrint(font, 40, 20 + ((40 - (font->texYSize - 30)) / 2), "Sorting Options");

		int printed = 0; // Print counter

		for (i = 0; i < max_items + 1; i++) {
			if (printed == FILES_PER_PAGE)
				break;

			if (selection < FILES_PER_PAGE || i > (selection - FILES_PER_PAGE)) {
				if (i == selection)
					G2D_DrawRect(0, 62 + (42 * printed), 480, 42, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
				
				intraFontSetStyle(font, 0.7f, config.dark_theme? WHITE : BLACK, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
				intraFontPrint(font, 20, 62 + ((42 - (font->texYSize - 30)) / 2) + (42 * printed), main_menu_items[i]);

				printed++;
			}
		}

		config.sort == 0? G2D_DrawImage(config.dark_theme? icon_radio_dark_on : icon_radio_on, (460 - width), 69) : 
			G2D_DrawImage(config.dark_theme? icon_radio_dark_off : icon_radio_off, (460 - width), 69);
		
		config.sort == 1? G2D_DrawImage(config.dark_theme? icon_radio_dark_on : icon_radio_on, (460 - width), 111) : 
			G2D_DrawImage(config.dark_theme? icon_radio_dark_off : icon_radio_off, (460 - width), 111);

		config.sort == 2? G2D_DrawImage(config.dark_theme? icon_radio_dark_on : icon_radio_on, (460 - width), 153) : 
			G2D_DrawImage(config.dark_theme? icon_radio_dark_off : icon_radio_off, (460 - width), 153);
		
		config.sort == 3? G2D_DrawImage(config.dark_theme? icon_radio_dark_on : icon_radio_on, (460 - width), 195) : 
			G2D_DrawImage(config.dark_theme? icon_radio_dark_off : icon_radio_off, (460 - width), 195);
		
		g2dFlip(G2D_VSYNC);

		Utils_ReadControls();

		if (Utils_IsButtonPressed(PSP_CTRL_CANCEL))
			break;
			
		if (Utils_IsButtonPressed(PSP_CTRL_DOWN))
			selection++;
		else if (Utils_IsButtonPressed(PSP_CTRL_UP))
			selection--;

		if (((Utils_IsButtonHeld(PSP_CTRL_LTRIGGER)) && (Utils_IsButtonPressed(PSP_CTRL_RTRIGGER))) || ((Utils_IsButtonHeld(PSP_CTRL_RTRIGGER)) && (Utils_IsButtonPressed(PSP_CTRL_LTRIGGER))))
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
		displayAbout = false;
}

static void Menu_DisplayAboutDialog(void) {
	int text_width = intraFontMeasureText(font, "CM File Manager PSP vX.X.X");
	int author_width = intraFontMeasureText(font, "Author: Joel16");

	G2D_DrawRect(0, 20, 480, 252, G2D_RGBA(0, 0, 0, config.dark_theme? 50: 80));

	G2D_DrawImage(config.dark_theme? dialog_dark : dialog, ((480 - dialog->w) / 2), ((272 - dialog->h) / 2));

	intraFontSetStyle(font, 0.7f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
	intraFontPrint(font, ((480 - dialog->w) / 2) + 10, ((272 - dialog->h) / 2) + 20, "About");

	intraFontSetStyle(font, 0.7f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
	intraFontPrintf(font, ((480 - (text_width)) / 2), ((272 - dialog->h) / 2) + 50, "CM File Manager PSP v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
	intraFontPrint(font, ((480 - (author_width)) / 2), ((272 - dialog->h) / 2) + 50 + 16, "Author: Joel16");

	intraFontSetStyle(font, 0.7f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
	G2D_DrawRect((409 - (intraFontMeasureText(font, "OK"))) - 5, (180 - (font->texYSize - 20)) - 5, intraFontMeasureText(font, "OK") + 10, (font->texYSize - 10) + 10, 
		config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	intraFontPrint(font, 409 - (intraFontMeasureText(font, "OK")), (182 - (font->texYSize - 30)), "OK");
}

static void Menu_ControlSupportDialog(void) {
	if ((Utils_IsButtonPressed(PSP_CTRL_ENTER)) || (Utils_IsButtonPressed(PSP_CTRL_CANCEL)))
		displaySupport = false;
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
	int selection = 0, max_items = 4, i = 0;

	const char *main_menu_items[] = {
		"Sorting options",
		"Dark theme",
		"Auto USB mount",
		"Support",
		"About",
	};

	displayAbout = false;

	int width = icon_toggle_on->w;
	int height = icon_toggle_on->h;

	while (1) {
		g2dClear(config.dark_theme? BLACK_BG : WHITE);
		G2D_DrawRect(0, 0, 480, 20, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
		G2D_DrawRect(0, 20, 480, 42, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);

		StatusBar_DisplayTime();

		G2D_DrawImage(icon_back, 5, 25);
		intraFontSetStyle(font, 0.7f, WHITE, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
		intraFontPrint(font, 40, 20 + ((40 - (font->texYSize - 30)) / 2), "Settings");

		int printed = 0; // Print counter

		for (i = 0; i < max_items + 1; i++) {
			if (printed == FILES_PER_PAGE)
				break;

			if (selection < FILES_PER_PAGE || i > (selection - FILES_PER_PAGE)) {
				if (i == selection)
					G2D_DrawRect(0, 62 + (42 * printed), 480, 42, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

				intraFontSetStyle(font, 0.7f, config.dark_theme? WHITE : BLACK, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
				intraFontPrint(font, 20, 62 + ((42 - (font->texYSize - 30)) / 2) + (42 * printed), main_menu_items[i]);

				printed++;
			}
		}

		if (config.dark_theme)
			G2D_DrawImage(config.dark_theme? icon_toggle_dark_on : icon_toggle_off, 455 - width, 104 + ((42 - height) / 2));
		else
			G2D_DrawImage(config.dark_theme? icon_toggle_on : icon_toggle_off, 455 - width,  104 + ((42 - height) / 2));

		if (config.dark_theme)
			G2D_DrawImage(config.auto_usb_mount? icon_toggle_dark_on : icon_toggle_off, 455 - width, 146 + ((42 - height) / 2));
		else
			G2D_DrawImage(config.auto_usb_mount? icon_toggle_on : icon_toggle_off, 455 - width,  146 + ((42 - height) / 2));

		if (displayAbout)
			Menu_DisplayAboutDialog();
		else if (displaySupport)
			Menu_DisplaySupportDialog();

		g2dFlip(G2D_VSYNC);

		Utils_ReadControls();

		if (((Utils_IsButtonHeld(PSP_CTRL_LTRIGGER)) && (Utils_IsButtonPressed(PSP_CTRL_RTRIGGER))) || ((Utils_IsButtonHeld(PSP_CTRL_RTRIGGER)) && (Utils_IsButtonPressed(PSP_CTRL_LTRIGGER))))
			Screenshot_Capture();

		if (displayAbout)
			Menu_ControlAboutDialog();
		else if (displaySupport)
			Menu_ControlSupportDialog();
		else {
			if ((Utils_IsButtonPressed(PSP_CTRL_CANCEL)) || (Utils_IsButtonPressed(PSP_CTRL_START)))
				break;
			
			if (Utils_IsButtonPressed(PSP_CTRL_DOWN))
				selection++;
			else if (Utils_IsButtonPressed(PSP_CTRL_UP))
				selection--;

			Utils_SetMax(&selection, 0, max_items);
			Utils_SetMin(&selection, max_items, 0);

			if (Utils_IsButtonPressed(PSP_CTRL_ENTER)) {
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
	}

	MENU_STATE = MENU_STATE_HOME;
}
