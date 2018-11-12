#include <stdbool.h>

#include "common.h"

void OSL_StartDrawing(void) {
	oslStartDrawing();
	oslSetBilinearFilter(1);
}

void OSL_EndDrawing(void) {
	oslSetBilinearFilter(0);
	oslEndDrawing();
	oslEndFrame();
	oslSyncFrame();
}

void OSL_DrawFillRect(int x, int y, int w, int h, OSL_COLOR color) {
	return oslDrawFillRect(x, y, x + w, y + h, color);
}

void OSL_DisplayKeyboard(char *descStr, char *initialStr, char *text) {
	int skip = 0;
	bool done = false;

	oslInitOsk(descStr, initialStr, 256, 1, -1);

	while(!osl_quit && !done) {
		if (!skip) {
			oslStartDrawing();
			oslClearScreen(RGBA(39, 50, 56, 255));

			if (oslOskIsActive())
				oslDrawOsk();
			if (oslGetOskStatus() == PSP_UTILITY_DIALOG_NONE) {
				if (oslOskGetResult() == OSL_OSK_CANCEL) {
					strcpy(text, "");
					done = true;
				}
				else {
					oslOskGetText(text);
					done = true;
				}
				oslEndOsk();
			}
			oslEndDrawing();
		}
		oslEndFrame();
		skip = oslSyncFrame();
	}
}
