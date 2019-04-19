#include <psptypes.h>
#include <sys/time.h>
#include <psprtc.h>
#include <malloc.h>
#include <stdlib.h>

#include "common.h"
#include "config.h"
#include "fs.h"
#include "glib2d_helper.h"
#include "textures.h"
#include "utils.h"

#define BYTES_PER_PIXEL 4
#define TRANSPARENT_COLOR 0xFFFFFFFF

static char album[1024][512];
static int count = 0, selection = 0;
static g2dTexture *image;
static float width = 0, height = 0, scale_factor = 1.0f;
static int degrees = 0, pos_x = 0, pos_y = 0;
static bool vertical_flip = false, horizantal_flip = false;

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
				|| (!strncasecmp(FS_GetFileExt(entries[i].d_name), "pcx", 3)) || (!strncasecmp(FS_GetFileExt(entries[i].d_name), "png", 3)) || (!strncasecmp(FS_GetFileExt(entries[i].d_name), "pgm", 3)) 
				|| (!strncasecmp(FS_GetFileExt(entries[i].d_name), "ppm", 3)) || (!strncasecmp(FS_GetFileExt(entries[i].d_name), "tga", 3))) {
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
	degrees = 0;
	pos_x = 0;
	pos_y = 0;
	vertical_flip = false;
	horizantal_flip = false;

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

static void Gallery_DrawImage(float x, float y, float w, float h, float zoom_factor, int angle) {
	g2dBeginRects(image); {
		g2dSetCoordMode(G2D_CENTER);
		g2dSetScaleWH(width * zoom_factor, height * zoom_factor);
		g2dSetCoordXY(x - (pos_x * zoom_factor - pos_x) / 2, y - (pos_y * zoom_factor - pos_y) / 2);
		g2dSetRotation(angle);
		g2dAdd();
	}
	g2dEnd();
}

static void Gallery_DisplaySupportDialog(void) {
	G2D_DrawRect(0, 0, 480, 272, G2D_RGBA(0, 0, 0, config.dark_theme? 50: 80));

	G2D_DrawImage(config.dark_theme? properties_dialog_dark : properties_dialog, 131, 32);
	intraFontSetStyle(font, 0.7f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);

	G2D_DrawRect((340 - intraFontMeasureText(font, "OK")) - 5, (230 - (font->texYSize - 6)) - 5, intraFontMeasureText(font, "OK") + 10, (font->texYSize - 6) + 10, 
		config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

	intraFontPrint(font, 138, 50, "Support");
	intraFontPrint(font, 340 - intraFontMeasureText(font, "OK"), 230 - (font->texYSize - 20), "OK");

	intraFontSetStyle(font, 0.7f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
	intraFontPrint(font, 140, 74, "DPAD Up: Zoom in");
	intraFontPrint(font, 140, 90, "DPAD Down: Zoom out");
	intraFontPrint(font, 140, 106, "DPAD Left: Previous image");
	intraFontPrint(font, 140, 122, "DPAD Right: Next image");
	intraFontPrint(font, 140, 138, "Triangle: Vertical flip");
	intraFontPrint(font, 140, 154, "Square: Horizantal flip");
	intraFontPrint(font, 140, 170, "R: Rotate clockwise");
	intraFontPrint(font, 140, 186, "L: Rotate anti-clockwise");
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
	degrees = 0;
	pos_x = 0;
	pos_y = 0;
	vertical_flip = false;
	horizantal_flip = false;

	bool display_support = false;

	u64 last = 0;
	u32 tick = sceRtcGetTickResolution();
	sceRtcGetCurrentTick(&last);

	while (1) {
		u64 current = 0;
		sceRtcGetCurrentTick(&current);

		float delta_time = (current - last) / (float)tick;
		last = current;

		g2dClear(G2D_RGBA(33, 39, 43, 255));
		Gallery_DrawImage(G2D_SCR_W / 2, G2D_SCR_H / 2, width, height, zoom_factor, degrees);
		
		if (display_support)
			Gallery_DisplaySupportDialog();
		
		g2dFlip(G2D_VSYNC);

		Utils_ReadControls();
		
		if (display_support) {
			if ((Utils_IsButtonPressed(PSP_CTRL_ENTER)) || (Utils_IsButtonPressed(PSP_CTRL_CANCEL)) || (Utils_IsButtonPressed(PSP_CTRL_SELECT)))
				display_support = false;
		}
		else {
			if (Utils_IsButtonPressed(PSP_CTRL_SELECT))
				display_support = true;

			if (Utils_IsButtonPressed(PSP_CTRL_LTRIGGER)) {
				degrees -= 90;

				if (degrees < 0)
					degrees = 270;
			}
			else if (Utils_IsButtonPressed(PSP_CTRL_RTRIGGER)) {
				degrees += 90;

				if (degrees > 270)
					degrees = 0;
			}

			// Flip horizantally
			if (Utils_IsButtonPressed(PSP_CTRL_SQUARE)) {
				horizantal_flip = !horizantal_flip;
				width = -width;
			}

			// Flip vertically
			if (Utils_IsButtonPressed(PSP_CTRL_TRIANGLE)) {
				vertical_flip = !vertical_flip;
				height = -height;
			}

			if (Utils_IsButtonHeld(PSP_CTRL_UP)) {
				zoom_factor += 0.5f * delta_time;

				if (zoom_factor > 2.0f)
					zoom_factor = 2.0f;
			}
			else if (Utils_IsButtonHeld(PSP_CTRL_DOWN)) {
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

			if ((degrees == 0) || (degrees == 180)) {
				Utils_SetMax(&pos_x, horizantal_flip? -width : width, horizantal_flip? -width : width);
				Utils_SetMin(&pos_x, horizantal_flip? width : -width, horizantal_flip? width : -width);
				Utils_SetMax(&pos_y, vertical_flip? -height : height, vertical_flip? -height : height);
				Utils_SetMin(&pos_y, vertical_flip? height : -height, vertical_flip? height : -height);
			}
			else {
				Utils_SetMax(&pos_x, vertical_flip? -height : height, vertical_flip? -height : height);
				Utils_SetMin(&pos_x, vertical_flip? height : -height, vertical_flip? height : -height);
				Utils_SetMax(&pos_y, horizantal_flip? -width : width, horizantal_flip? -width : width);
				Utils_SetMin(&pos_y, horizantal_flip? width : -width, horizantal_flip? width : -width);
			}

			if (Utils_IsButtonPressed(PSP_CTRL_LEFT))
				Gallery_HandleNext(false);
			else if (Utils_IsButtonPressed(PSP_CTRL_RIGHT))
				Gallery_HandleNext(true);

			if (Utils_IsButtonPressed(PSP_CTRL_CANCEL))
				break;
		}
	}

	g2dTexFree(&image);
	MENU_STATE = MENU_STATE_HOME;
}
