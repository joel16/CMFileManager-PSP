#include <stdbool.h>

#include "common.h"
#include "fs.h"
#include "osl_helper.h"
#include "utils.h"

static char album[1024][512];
static int count = 0, selection = 0;
static OSL_IMAGE *image;
static int width = 0, height = 0;

static int Gallery_GetImageList(void) {
	SceUID dir = 0;

	if (R_SUCCEEDED(dir = sceIoDopen(cwd))) {
		int entryCount = 0, i = 0;
		SceIoDirent *entries = (SceIoDirent *)calloc(MAX_FILES, sizeof(SceIoDirent));

		while (sceIoDread(dir, &entries[entryCount]) > 0)
			entryCount++;

		sceIoDclose(dir);
		qsort(entries, entryCount, sizeof(SceIoDirent), Utils_Alphasort);

		for (i = 0; i < entryCount; i++) {
			if ((!strncasecmp(FS_GetFileExt(entries[i].d_name), "png", 3)) || (!strncasecmp(FS_GetFileExt(entries[i].d_name), "jpg", 3)) || 
				(!strncasecmp(FS_GetFileExt(entries[i].d_name), "gif", 3))) {
				strcpy(album[count], cwd);
				strcpy(album[count] + strlen(album[count]), entries[i].d_name);
				count++;
			}
		}

		free(entries);
	}
	else {
		sceIoDclose(dir);
		return dir;
	}

	return 0;
}

static int Gallery_GetCurrentIndex(char *path) {
	int i = 0;
	for(i = 0; i < count; ++i) {
		if (!strcmp(album[i], path))
			return i;
	}

	return 0;
}

static void Gallery_HandleNext(bool forward) {
	if (forward)
		selection++;
	else
		selection--;

	Utils_SetMax(&selection, 0, (count - 1));
	Utils_SetMin(&selection, (count - 1), 0);

	oslDeleteImage(image);
	selection = Gallery_GetCurrentIndex(album[selection]);

	image = oslLoadImageFile(album[selection], OSL_IN_RAM, OSL_PF_8888);
	width = oslGetImageWidth(image);
	height = oslGetImageHeight(image);
}

void Gallery_DisplayImage(char *path) {
	Gallery_GetImageList();
	selection = Gallery_GetCurrentIndex(path);
	image = oslLoadImageFile(path, OSL_IN_RAM, OSL_PF_8888);
	width = oslGetImageWidth(image);
	height = oslGetImageHeight(image);

	while (!osl_quit) {
		OSL_StartDrawing();
		oslClearScreen(RGBA(33, 39, 43, 255));

		oslDrawImageXY(image, (480 - width) / 2, (272 - height) / 2);

		OSL_EndDrawing();
		oslReadKeys();

		if ((osl_keys->pressed.left) || (osl_keys->pressed.L))
			Gallery_HandleNext(false);
		else if ((osl_keys->pressed.right) || (osl_keys->pressed.R))
			Gallery_HandleNext(true);

		if (osl_keys->pressed.circle)
			break;
	}

	oslDeleteImage(image);
	MENU_STATE = MENU_STATE_HOME;
}
