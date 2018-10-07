#include "common.h"
#include "config.h"
#include "osl_helper.h"
#include "textures.h"

void ProgressBar_DisplayProgress(char *msg, char *src, u32 offset, u32 size) {
	oslStartDrawing();
	int text_width = oslGetStringWidth(src);

	oslDrawImageXY(config_dark_theme? dialog_dark : dialog, ((480 - oslGetImageWidth(dialog)) / 2), ((272 - oslGetImageHeight(dialog)) / 2));

	oslIntraFontSetStyle(font, 0.6f, config_dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawString(((480 - oslGetImageWidth(dialog)) / 2) + 8, ((272 - oslGetImageHeight(dialog)) / 2) + 6, msg);

	oslIntraFontSetStyle(font, 0.6f, config_dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawString(((480 - (text_width)) / 2), ((272 - oslGetImageHeight(dialog)) / 2) + 30, src);

	OSL_DawFillRect(((480 - oslGetImageWidth(dialog)) / 2) + 20, ((272 - oslGetImageHeight(dialog)) / 2) + 60, 
		272, 4, config_dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	OSL_DawFillRect(((480 - oslGetImageWidth(dialog)) / 2) + 20, ((272 - oslGetImageHeight(dialog)) / 2) + 60, 
		((double)offset / (double)size * 272.0), 4, config_dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);

	oslSyncFrame();
}
