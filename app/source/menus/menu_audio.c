#include <pspaudiolib.h>
#include <malloc.h>
#include <stdio.h>

#include "audio.h"
#include "common.h"
#include "config.h"
#include "glib2d_helper.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

typedef enum {
	MUSIC_STATE_NONE,   // 0
	MUSIC_STATE_REPEAT, // 1
	MUSIC_STATE_SHUFFLE // 2
} Music_State;

static int state = 0;

static char *Menu_ConvertSecondsToString(char *string, u64 seconds) {
	int h = 0, m = 0, s = 0;
	h = (seconds / 3600);
	m = (seconds - (3600 * h)) / 60;
	s = (seconds - (3600 * h) - (m * 60));

	if (h > 0)
		snprintf(string, 35, "%02d:%02d:%02d", h, m, s);
	else
		snprintf(string, 35, "%02d:%02d", m, s);

	return string;
}

void Menu_PlayAudio(const char *path) {
	Audio_Init(path);
	pspAudioSetChannelCallback(0, Audio_Decode, NULL);

	u64 length = Audio_GetLengthSeconds(path);
	char *position_time = malloc(35);
	char *length_time = malloc(35);
	float length_time_width = 0;

	if (length) {
		length_time = Menu_ConvertSecondsToString(length_time, Audio_GetLengthSeconds(path));
		intraFontSetStyle(font, 0.6f, WHITE, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
		length_time_width = intraFontMeasureText(font, length_time);
	}

	while(playing) {
		g2dClear(config.dark_theme? BLACK_BG : WHITE);
		G2D_DrawImage(default_artwork_blur, 0, 0);
		G2D_DrawRect(0, 0, 480, 20, G2D_RGBA(97, 97, 97, 255));

		G2D_DrawImage(icon_back, 5, 25);
		StatusBar_DisplayTime();

		intraFontSetStyle(font, 0.7f, WHITE, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
		intraFontPrint(font, 40, 22 + ((40 - (font->texYSize - 30)) / 2), strupr(Utils_Basename(path)));

		G2D_DrawRect(0, 62, 200, 200, G2D_RGBA(97, 97, 97, 255));
		G2D_DrawImage(default_artwork, 0, 62);

		G2D_DrawRect(205, 62, 275, 200, G2D_RGBA(45, 48, 50, 255)); // Draw info box (outer)
		G2D_DrawRect(210, 67, 265, 190, G2D_RGBA(46, 49, 51, 255)); // Draw info box (inner)

		if (!Audio_IsPaused())
			G2D_DrawImage(btn_pause, 205 + ((275 - btn_pause->w) / 2), 62 + ((200 - btn_pause->h) / 2)); // Playing
		else
			G2D_DrawImage(btn_play, 205 + ((275 - btn_play->w) / 2), 62 + ((200 - btn_play->h) / 2)); // Paused

		G2D_DrawImage(btn_rewind, 205 + ((275 - btn_rewind->w) / 2) - 68, 62 + ((200 - btn_rewind->h) / 2));
		G2D_DrawImage(btn_forward, 205 + ((275 - btn_forward->w) / 2) + 68, 62 + ((200 - btn_forward->h) / 2));

		G2D_DrawImage(state == MUSIC_STATE_SHUFFLE? btn_shuffle_overlay : btn_shuffle, 205 + ((275 - btn_shuffle->w) / 2) - 45, 62 + ((200 - btn_shuffle->h) / 2) + 50);
		G2D_DrawImage(state == MUSIC_STATE_REPEAT? btn_repeat_overlay : btn_repeat, 205 + ((275 - btn_repeat->w) / 2) + 45, 62 + ((200 - btn_repeat->h) / 2) + 50);

		if (length) {
			intraFontSetStyle(font, 0.6f, WHITE, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
			position_time = Menu_ConvertSecondsToString(position_time, Audio_GetPositionSeconds(path));
			intraFontPrint(font, 230, 240, position_time);
			intraFontPrint(font, 455 - length_time_width, 240, length_time);
		}

		// Progress bar
		if (Audio_GetPosition() != -1) {
			G2D_DrawRect(230, 245, 225, 2, G2D_RGBA(97, 97, 97, 150));
			G2D_DrawRect(230, 245, (((double)Audio_GetPosition()/(double)Audio_GetLength()) * 225.0), 2, WHITE);
		}

		g2dFlip(G2D_VSYNC);

		Utils_ReadControls();

		if (Utils_IsButtonPressed(PSP_CTRL_ENTER))
			Audio_Pause();

		if (Utils_IsButtonPressed(PSP_CTRL_CANCEL))
			Audio_Stop();
	}

	free(length_time);
	free(position_time);

	Audio_Term();
}
