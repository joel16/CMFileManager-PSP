#include "common.h"
#include "config.h"
#include "osl_helper.h"
#include "textures.h"

static char src_resized[41];

void ProgressBar_DisplayProgress(char *msg, char *src, u32 offset, u32 size) {
	oslStartDrawing();
	snprintf(src_resized, 41, "%.40s", src);
	int text_width = oslGetStringWidth(src_resized);

	oslDrawImageXY(config.dark_theme? dialog_dark : dialog, ((480 - oslGetImageWidth(dialog)) / 2), ((272 - oslGetImageHeight(dialog)) / 2));

	oslIntraFontSetStyle(font, 0.6f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawString(((480 - oslGetImageWidth(dialog)) / 2) + 8, ((272 - oslGetImageHeight(dialog)) / 2) + 16, msg);

	oslIntraFontSetStyle(font, 0.6f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	oslDrawString(((480 - (text_width)) / 2), ((272 - oslGetImageHeight(dialog)) / 2) + 40, src_resized);

	OSL_DrawFillRect(((480 - oslGetImageWidth(dialog)) / 2) + 20, ((272 - oslGetImageHeight(dialog)) / 2) + 70, 
		318, 4, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	OSL_DrawFillRect(((480 - oslGetImageWidth(dialog)) / 2) + 20, ((272 - oslGetImageHeight(dialog)) / 2) + 70, 
		((double)offset / (double)size * 318.0), 4, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);

	oslSyncFrame();
}
