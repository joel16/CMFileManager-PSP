#include <stdbool.h>

#include "common.h"
#include "config.h"

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

void OSL_DawFillRect(int x, int y, int w, int h, OSL_COLOR color) {
	return oslDrawFillRect(x, y, x + w, y + h, color);
}

void OSL_DisplayKeyboard(char *descStr, char *initialStr, char *text) {
	int skip = 0;
	bool done = false;

	oslInitOsk(descStr, initialStr, 256, 1, -1);

	while(!osl_quit && !done) {
		if (!skip) {
			oslStartDrawing();
			OSL_DawFillRect(0, 0, 480, 20, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
			OSL_DawFillRect(0, 20, 480, 42, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
			if (oslOskIsActive())
				oslDrawOsk();
			if (oslGetOskStatus() == PSP_UTILITY_DIALOG_NONE) {
				if (oslOskGetResult() == OSL_OSK_CANCEL) {
					strcpy(text, "");
					done = 1;
				}
				else {
					oslOskGetText(text);
					done = 1;
				}
				oslEndOsk();
			}
			oslEndDrawing();
		}
		oslEndFrame();
		skip = oslSyncFrame();
	}
}
