#include <stdio.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "glib2d_helper.h"
#include "status_bar.h"
#include "textures.h"

static char src_resized[41];

void ProgressBar_DisplayProgress(char *msg, char *src, u32 offset, u32 size) {
	g2dClear(config.dark_theme? BLACK_BG : WHITE);
	G2D_DrawRect(0, 0, 480, 20, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
	G2D_DrawRect(0, 20, 480, 42, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
	G2D_DrawImage(icon_nav_drawer, 5, 25);

	StatusBar_DisplayTime();
	Dirbrowse_DisplayFiles();

	G2D_DrawRect(0, 20, 480, 252, G2D_RGBA(0, 0, 0, config.dark_theme? 50: 80));
	
	snprintf(src_resized, 41, "%.40s", src);
	int text_width = intraFontMeasureText(font, src_resized);

	G2D_DrawImage(config.dark_theme? dialog_dark : dialog, ((480 - dialog->w) / 2), ((272 - dialog->h) / 2));

	intraFontSetStyle(font, 0.7f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
	intraFontPrint(font, ((480 - dialog->w) / 2) + 10, ((272 - dialog->h) / 2) + 20, msg);

	intraFontSetStyle(font, 0.7f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
	intraFontPrint(font, ((480 - (text_width)) / 2), ((272 - dialog->h) / 2) + 50, src_resized);

	G2D_DrawRect(((480 - dialog->w) / 2) + 20, ((272 - dialog->h) / 2) + 70, 
		318, 4, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	G2D_DrawRect(((480 - dialog->w) / 2) + 20, ((272 - dialog->h) / 2) + 70, 
		((double)offset / (double)size * 318.0), 4, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);

	g2dFlip(G2D_VSYNC);
}
