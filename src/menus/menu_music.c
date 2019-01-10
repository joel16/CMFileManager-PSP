#include <stdbool.h>

#include "common.h"
#include "config.h"
#include "osl_helper.h"
#include "screenshot.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

#define MUSIC_GENRE_COLOUR      RGBA(97, 97, 97, 255)
#define MUSIC_STATUS_BG_COLOUR  RGBA(43, 53, 61, 255)
#define MUSIC_SEPARATOR_COLOUR  RGBA(34, 41, 48, 255)

typedef enum {
	MUSIC_STATE_NONE,   // 0
	MUSIC_STATE_REPEAT, // 1
	MUSIC_STATE_SHUFFLE // 2
} Music_State;

static OSL_SOUND *audio;
static int state = 0;

void Menu_PlayMusic(char *path){
	audio = oslLoadSoundFile(path, OSL_FMT_STREAM);
	oslPlaySound(audio, 0);

	bool isPlaying = true;

	int btn_width = oslGetImageWidth(btn_play);
	int btn_height = oslGetImageHeight(btn_play);

	while (!osl_quit) {
		OSL_StartDrawing();
		oslClearScreen(RGBA(54, 68, 76, 255));
		OSL_DrawFillRect(0, 0, 480, 20, MUSIC_GENRE_COLOUR);
		OSL_DrawFillRect(0, 61, 480, 1, MUSIC_SEPARATOR_COLOUR);

		oslDrawImageXY(icon_back, 5, 25);
		StatusBar_DisplayTime();

		OSL_DrawFillRect(0, 62, 200, 200, MUSIC_GENRE_COLOUR);
		oslDrawImageXY(default_artwork, 0, 62);

		OSL_DrawFillRect(205, 62, 275, 200, RGBA(45, 48, 50, 255)); // Draw info box (outer)
		OSL_DrawFillRect(210, 67, 265, 190, RGBA(46, 49, 51, 255)); // Draw info box (inner)

		if (isPlaying)
			oslDrawImageXY(btn_pause, 205 + ((275 - btn_width) / 2), 62 + ((200 - btn_height) / 2)); // Playing
		else
			oslDrawImageXY(btn_play, 205 + ((275 - btn_width) / 2), 62 + ((200 - btn_height) / 2)); // Paused

		oslDrawImageXY(btn_rewind, 205 + ((275 - btn_width) / 2) - 68, 62 + ((200 - btn_height) / 2)); // Paused
		oslDrawImageXY(btn_forward, 205 + ((275 - btn_width) / 2) + 68, 62 + ((200 - btn_height) / 2)); // Paused

		oslDrawImageXY(state == MUSIC_STATE_SHUFFLE? btn_shuffle_overlay : btn_shuffle, 205 + ((275 - btn_width) / 2) - 45, 62 + ((200 - btn_height) / 2) + 50); // Paused
		oslDrawImageXY(state == MUSIC_STATE_REPEAT? btn_repeat_overlay : btn_repeat, 205 + ((275 - btn_width) / 2) + 45, 62 + ((200 - btn_height) / 2) + 50); // Paused

		OSL_EndDrawing();
		oslReadKeys();

		if (osl_keys->pressed.value & OSL_KEYMASK_ENTER) {
			if (isPlaying) {
				isPlaying = false;
				oslPauseSound(audio, -1);
			}
			else {
				isPlaying = true;
				oslPauseSound(audio, -1);
			}
		}

		if (osl_keys->pressed.square) {
			if (state == MUSIC_STATE_REPEAT)
				state = MUSIC_STATE_NONE;
			else
				state = MUSIC_STATE_REPEAT;
		}
		else if (osl_keys->pressed.triangle) {
			if (state == MUSIC_STATE_SHUFFLE)
				state = MUSIC_STATE_NONE;
			else
				state = MUSIC_STATE_SHUFFLE;
		}

		if (osl_keys->pressed.value & OSL_KEYMASK_CANCEL)
			break;
	}

	oslStopSound(audio);
	oslDeleteSound(audio);
	MENU_STATE = MENU_STATE_HOME;
}