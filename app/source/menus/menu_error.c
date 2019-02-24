#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "glib2d_helper.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

void Menu_DisplayError(char *msg, int ret) {
	char *result = malloc(64);
	if (ret != 0)
		snprintf(result, 64, "Ret: %08X\n", ret);

	int msg_width = intraFontMeasureText(font, msg);
	int result_width = intraFontMeasureText(font, result);

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
		intraFontPrint(font, ((480 - dialog->w) / 2) + 10, ((272 - dialog->h) / 2) + 20, "Error");

		intraFontSetStyle(font, 0.7f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
		intraFontPrint(font, ((480 - (msg_width)) / 2), ((272 - dialog->h) / 2) + 50, msg);

		if (ret != 0)
			intraFontPrint(font, ((480 - (result_width)) / 2), ((272 - dialog->h) / 2) + 50 + 16, result);

		intraFontSetStyle(font, 0.7f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
		G2D_DrawRect((409 - (intraFontMeasureText(font, "OK"))) - 5, (180 - (font->texYSize - 20)) - 5, intraFontMeasureText(font, "OK") + 10, (font->texYSize - 10) + 10, 
			config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
		intraFontPrint(font, 409 - (intraFontMeasureText(font, "OK")), (182 - (font->texYSize - 30)), "OK");

		g2dFlip(G2D_VSYNC);

		Utils_ReadControls();

		if ((Utils_IsButtonPressed(PSP_CTRL_ENTER)) || (Utils_IsButtonPressed(PSP_CTRL_CANCEL)))
			break;
	}

	free(result);
}
