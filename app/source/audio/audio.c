#include <pspaudiolib.h>
#include <pspthreadman.h>
#include <string.h>

#include "fs.h"

#include "flac.h"
#include "mp3.h"
#include "ogg.h"
#include "wav.h"
#include "xm.h"

bool playing = true, paused = false;

enum Audio_FileType {
	FILE_TYPE_NONE = 0,
	FILE_TYPE_FLAC = 1,
	FILE_TYPE_MP3 = 2,
	FILE_TYPE_OGG = 3,
	FILE_TYPE_WAV = 4,
	FILE_TYPE_XM = 5
};

static enum Audio_FileType file_type = FILE_TYPE_NONE;

void Audio_Init(const char *path) {
	pspAudioInit();
	playing = true;
	paused = false;

	if (!strncasecmp(FS_GetFileExt(path), "flac", 4))
		file_type = FILE_TYPE_FLAC;
	else if (!strncasecmp(FS_GetFileExt(path), "mp3", 3))
		file_type = FILE_TYPE_MP3;
	else if (!strncasecmp(FS_GetFileExt(path), "ogg", 3))
		file_type = FILE_TYPE_OGG;
	else if (!strncasecmp(FS_GetFileExt(path), "wav", 3))
		file_type = FILE_TYPE_WAV;
	else if (!strncasecmp(FS_GetFileExt(path), "xm", 2))
		file_type = FILE_TYPE_XM;

	switch(file_type) {
		case FILE_TYPE_FLAC:
			FLAC_Init(path);
			break;

		case FILE_TYPE_MP3:
			MP3_Init(path);
			break;

		case FILE_TYPE_OGG:
			OGG_Init(path);
			break;

		case FILE_TYPE_WAV:
			WAV_Init(path);
			break;

		case FILE_TYPE_XM:
			XM_Init(path);
			break;

		default:
			break;
	}
}

void Audio_Decode(void *buf, unsigned int length, void *userdata) {
	if (playing == false || paused == true) {
		short *buf_short = (short *)buf;
		unsigned int count;
		for (count = 0; count < length * 2; count++)
			*(buf_short + count) = 0;
	} 
	else {
		switch(file_type) {
			case FILE_TYPE_FLAC:
				FLAC_Decode(buf, length, userdata);
				break;

			case FILE_TYPE_MP3:
				MP3_Decode(buf, length, userdata);
				break;

			case FILE_TYPE_OGG:
				OGG_Decode(buf, length, userdata);
				break;

			case FILE_TYPE_WAV:
				WAV_Decode(buf, length, userdata);
				break;

			case FILE_TYPE_XM:
				XM_Decode(buf, length, userdata);
				break;

			default:
				break;
		}
	}
}

bool Audio_IsPaused(void) {
	return paused;
}

void Audio_Pause(void) {
	paused = !paused;
}

void Audio_Stop(void) {
	playing = !playing;
}

u64 Audio_GetPosition(void) {
	u64 position = -1;

	switch(file_type) {
		case FILE_TYPE_FLAC:
			position = FLAC_GetPosition();
			break;

		case FILE_TYPE_MP3:
			position = MP3_GetPosition();
			break;

		case FILE_TYPE_OGG:
			position = OGG_GetPosition();
			break;

		case FILE_TYPE_WAV:
			position = WAV_GetPosition();
			break;

		case FILE_TYPE_XM:
			position = XM_GetPosition();
			break;

		default:
			break;
	}

	return position;
}

u64 Audio_GetLength(void) {
	u64 length = 0;

	switch(file_type) {
		case FILE_TYPE_FLAC:
			length = FLAC_GetLength();
			break;

		case FILE_TYPE_MP3:
			length = MP3_GetLength();
			break;

		case FILE_TYPE_OGG:
			length = OGG_GetLength();
			break;

		case FILE_TYPE_WAV:
			length = WAV_GetLength();
			break;

		case FILE_TYPE_XM:
			length = XM_GetLength();
			break;

		default:
			break;
	}

	return length;
}

u64 Audio_GetPositionSeconds(const char *path) {
	u64 seconds = -1;

	switch(file_type) {
		case FILE_TYPE_FLAC:
			seconds = FLAC_GetPositionSeconds(path);
			break;

		case FILE_TYPE_MP3:
			seconds = MP3_GetPositionSeconds(path);
			break;

		case FILE_TYPE_OGG:
			seconds = OGG_GetPositionSeconds(path);
			break;

		case FILE_TYPE_WAV:
			seconds = WAV_GetPositionSeconds(path);
			break;

		case FILE_TYPE_XM:
			seconds = XM_GetPositionSeconds(path);
			break;

		default:
			break;
	}

	return seconds;
}

u64 Audio_GetLengthSeconds(const char *path) {
	u64 seconds = 0;

	switch(file_type) {
		case FILE_TYPE_FLAC:
			seconds = FLAC_GetLengthSeconds(path);
			break;

		case FILE_TYPE_MP3:
			seconds = MP3_GetLengthSeconds(path);
			break;

		case FILE_TYPE_OGG:
			seconds = OGG_GetLengthSeconds(path);
			break;

		case FILE_TYPE_WAV:
			seconds = WAV_GetLengthSeconds(path);
			break;

		case FILE_TYPE_XM:
			seconds = XM_GetLengthSeconds(path);
			break;

		default:
			break;
	}

	return seconds;
}

void Audio_Term(void) {
	switch(file_type) {
		case FILE_TYPE_FLAC:
			FLAC_Term();
			break;

		case FILE_TYPE_MP3:
			MP3_Term();
			break;

		case FILE_TYPE_OGG:
			OGG_Term();
			break;

		case FILE_TYPE_WAV:
			WAV_Term();
			break;

		case FILE_TYPE_XM:
			XM_Term();
			break;

		default:
			break;
	}

	playing = true;
	paused = false;

	pspAudioSetChannelCallback(0, NULL, NULL); // Clear channel callback
	pspAudioEndPre();
	sceKernelDelayThread(50 * 1000);
	pspAudioEnd();
}
