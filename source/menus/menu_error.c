#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "osl_helper.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

void Menu_DisplayError(char *msg, int ret) {
	char *result = malloc(64);
	if (ret != 0)
		snprintf(result, 64, "Ret: %08X\n", ret);

	int msg_width = oslGetStringWidth(msg);
	int result_width = oslGetStringWidth(result);

	while (!osl_quit) {
		OSL_StartDrawing();
		oslClearScreen(config.dark_theme? BLACK_BG : WHITE);
		OSL_DrawFillRect(0, 0, 480, 20, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
		OSL_DrawFillRect(0, 20, 480, 42, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
		oslDrawImageXY(icon_nav_drawer, 5, 25);

		StatusBar_DisplayTime();
		Dirbrowse_DisplayFiles();

		oslDrawImageXY(config.dark_theme? dialog_dark : dialog, ((480 - oslGetImageWidth(dialog)) / 2), ((272 - oslGetImageHeight(dialog)) / 2));

		oslIntraFontSetStyle(font, 0.6f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
		oslDrawString(((480 - oslGetImageWidth(dialog)) / 2) + 6, ((272 - oslGetImageHeight(dialog)) / 2) + 6, "Error");

		oslIntraFontSetStyle(font, 0.6f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
		oslDrawString(((480 - (msg_width)) / 2), ((272 - oslGetImageHeight(dialog)) / 2) + 40, msg);

		if (ret != 0)
			oslDrawString(((480 - (result_width)) / 2), ((272 - oslGetImageHeight(dialog)) / 2) + 40 + 16, result);

		OSL_DrawFillRect((409 - (oslGetStringWidth("OK"))) - 5, (191 - (font->charHeight - 6)) - 5, oslGetStringWidth("OK") + 10, (font->charHeight - 6) + 10, 
			config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

		oslIntraFontSetStyle(font, 0.6f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
		oslDrawString(409 - (oslGetStringWidth("OK")), (191 - (font->charHeight - 6)), "OK");

		OSL_EndDrawing();
		oslReadKeys();

		if ((osl_keys->pressed.value & OSL_KEYMASK_ENTER) || (osl_keys->pressed.value & OSL_KEYMASK_CANCEL))
			break;
	}

	free(result);
}
