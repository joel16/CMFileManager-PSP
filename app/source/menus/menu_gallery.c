#include <psptypes.h>
#include <sys/time.h>
#include <psprtc.h>
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
static int pos_x = 0, pos_y = 0;

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
				|| (!strncasecmp(FS_GetFileExt(entries[i].d_name), "png", 3)) || (!strncasecmp(FS_GetFileExt(entries[i].d_name), "tga", 3))) {
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
	pos_x = 0;
	pos_y = 0;
	
	if (forward)
		selection++;
	else
		selection--;

	Utils_SetMax(&selection, 0, (count - 1));
	Utils_SetMin(&selection, (count - 1), 0);

	g2dTexFree(&image);
	selection = Gallery_GetCurrentIndex(album[selection]);

	image = g2dTexLoad(album[selection], G2D_SWIZZLE);

	if (image->h > 272) {
		scale_factor = (272.0f / image->h);
		width = image->w * scale_factor;
		height = image->h * scale_factor;
	}
	else {
		width = image->w;
		height = image->h;
	}
}

static void Gallery_DrawImage(float x, float y, float w, float h, float zoom_factor) {
	g2dBeginRects(image); {
		g2dSetScaleWH(width * zoom_factor, height * zoom_factor);
		g2dSetCoordXY(x - (pos_x * zoom_factor - pos_x) / 2, y - (pos_y * zoom_factor - pos_y) / 2);
		g2dAdd();
	}
	g2dEnd();
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

	float zoom_factor = 1.0f;
	pos_x = 0;
	pos_y = 0;

	u64 last = 0;
	u32 tick = sceRtcGetTickResolution();
	sceRtcGetCurrentTick(&last);

	while (1) {
		u64 current = 0;
		sceRtcGetCurrentTick(&current);

		float delta_time = (current - last) / (float)tick;
		last = current;

		g2dClear(G2D_RGBA(33, 39, 43, 255));
		Gallery_DrawImage((G2D_SCR_W - (width * zoom_factor)) / 2, (G2D_SCR_H - (height * zoom_factor)) / 2, width, height, zoom_factor);
		g2dFlip(G2D_VSYNC);

		Utils_ReadControls();

		if (Utils_IsButtonHeld(PSP_CTRL_RTRIGGER)) {
			zoom_factor += 0.5f * delta_time;

			if (zoom_factor > 2.0f)
				zoom_factor = 2.0f;
		}
		else if (Utils_IsButtonHeld(PSP_CTRL_LTRIGGER)) {
			zoom_factor -= 0.5f * delta_time;

			if (zoom_factor < 0.5f)
				zoom_factor = 0.5f;

			if (zoom_factor <= 1.0f) {
				pos_x = 0;
				pos_y = 0;
			}
		}

		if ((height * zoom_factor > 272) || (width * zoom_factor > 480)) {
			double velocity = 2 / zoom_factor;
			if (Utils_GetAnalogY() < -0.4f)
				pos_y -= ((velocity * zoom_factor) * delta_time * 1000);
			if (Utils_GetAnalogY() > 0.4f)
				pos_y += ((velocity * zoom_factor) * delta_time * 1000);
			if (Utils_GetAnalogX() < -0.4f)
				pos_x -= ((velocity * zoom_factor) * delta_time * 1000);
			if (Utils_GetAnalogX() > 0.4f)
				pos_x += ((velocity * zoom_factor) * delta_time * 1000);
		}

		Utils_SetMax(&pos_x, width, width);
		Utils_SetMin(&pos_x, -width, -width);
		Utils_SetMax(&pos_y, height, height);
		Utils_SetMin(&pos_y, -height, -height);

		if (Utils_IsButtonPressed(PSP_CTRL_LEFT))
			Gallery_HandleNext(false);
		else if (Utils_IsButtonPressed(PSP_CTRL_RIGHT))
			Gallery_HandleNext(true);

		if (Utils_IsButtonPressed(PSP_CTRL_CANCEL))
			break;
	}

	g2dTexFree(&image);
	MENU_STATE = MENU_STATE_HOME;
}
