#include <oslib/oslib.h>

#include "../common.h"
#include "../utils.h"

static OSL_IMAGE *image;

void Gallery_DisplayImage(char *path) {
	image = oslLoadImageFile(path, OSL_IN_RAM, OSL_PF_8888);
	int width = oslGetImageWidth(image);
	int height = oslGetImageHeight(image);

	while (!osl_quit) {
		oslStartDrawing();
		oslClearScreen(RGBA(33, 39, 43, 255));

		oslDrawImageXY(image, (480 - width) / 2, (272 - height) / 2);

		Utils_EndDrawing();
		oslReadKeys();

		if (osl_keys->pressed.circle)
			break;
	}

	oslDeleteImage(image);
	MENU_STATE = MENU_STATE_HOME;
}