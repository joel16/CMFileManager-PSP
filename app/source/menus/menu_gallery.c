#include <malloc.h>
#include <stdlib.h>

#include "common.h"
#include "fs.h"
#include "glib2d_helper.h"
#include "utils.h"

#define BYTES_PER_PIXEL 4
#define TRANSPARENT_COLOR 0xFFFFFFFF

static char album[1024][512];
static int count = 0, selection = 0;
static g2dTexture *image;
static float width = 0, height = 0, scale_factor = 1.0f;

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
			if ((!strncasecmp(FS_GetFileExt(entries[i].d_name), "bmp", 3)) || (!strncasecmp(FS_GetFileExt(entries[i].d_name), "gif", 3)) || (!strncasecmp(FS_GetFileExt(entries[i].d_name), "jpg", 3)) 
				|| (!strncasecmp(FS_GetFileExt(entries[i].d_name), "png", 3))) {
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

	g2dTexFree(&image);
	selection = Gallery_GetCurrentIndex(album[selection]);

	image = g2dTexLoad(album[selection], G2D_SWIZZLE);
	width = image->w;
	height = image->h;
}

void Gallery_DisplayImage(char *path) {
	Gallery_GetImageList();
	selection = Gallery_GetCurrentIndex(path);
	image = g2dTexLoad(path, G2D_SWIZZLE);

	if (image->h > 272) {
		scale_factor = (272.0f / image->h);
		width = image->w * scale_factor;
		height = image->h * scale_factor;
	}
	else {
		width = image->w;
		height = image->h;
	}

	while (1) {
		g2dClear(G2D_RGBA(33, 39, 43, 255));

		if (image->h > 272)
			G2D_DrawImageScale(image, (G2D_SCR_W - width) / 2, (G2D_SCR_H - height) / 2, width, height);
		else
			G2D_DrawImage(image, (G2D_SCR_W - width) / 2, (G2D_SCR_H - height) / 2);

		g2dFlip(G2D_VSYNC);

		Utils_ReadControls();

		if ((Utils_IsButtonPressed(PSP_CTRL_LEFT)) || (Utils_IsButtonPressed(PSP_CTRL_LTRIGGER)))
			Gallery_HandleNext(false);
		else if ((Utils_IsButtonPressed(PSP_CTRL_RIGHT)) || (Utils_IsButtonPressed(PSP_CTRL_RTRIGGER)))
			Gallery_HandleNext(true);

		if (Utils_IsButtonPressed(PSP_CTRL_CANCEL))
			break;
	}

	g2dTexFree(&image);
	MENU_STATE = MENU_STATE_HOME;
}
