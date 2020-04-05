#include <pspaudio.h>
#include <pspthreadman.h>
#include <string.h>

#include "fs.h"
#include "kernel_functions.h"
#include "audio/audio.h"
#include "audio/pspaudiolib_cm.h"

#include "audio/flac.h"
#include "audio/mp3.h"
#include "audio/ogg.h"
#include "audio/opus.h"
#include "audio/wav.h"
#include "audio/xm.h"

enum Audio_FileType {
	FILE_TYPE_NONE = 0,
	FILE_TYPE_FLAC = 1,
	FILE_TYPE_MP3 = 2,
	FILE_TYPE_OGG = 3,
	FILE_TYPE_OPUS = 4,
	FILE_TYPE_WAV = 5,
	FILE_TYPE_XM = 6
};

typedef struct {
	int (* init)(const char *path);
	u32 (* rate)(void);
	u8 (* channels)(void);
	void (* decode)(void *buf, unsigned int length, void *userdata);
	u64 (* position)(void);
	u64 (* length)(void);
	u64 (* seek)(u64 index);
	void (* term)(void);
} Audio_Decoder;

static enum Audio_FileType file_type = FILE_TYPE_NONE;
Audio_Metadata metadata = {0};
static Audio_Metadata empty_metadata = {0};
static Audio_Decoder decoder = {0}, empty_decoder = {0};
bool playing = true, paused = false;

static void Audio_Decode(void *buf, unsigned int length, void *userdata) {
	if (playing == false || paused == true) {
		short *buf_short = (short *)buf;
		unsigned int count;
		for (count = 0; count < length * 2; count++)
			*(buf_short + count) = 0;
	} 
	else
		(* decoder.decode)(buf, length, userdata);
}

void Audio_Init(const char *path) {
	playing = true;
	paused = false;

	if (!strncasecmp(FS_GetFileExt(path), "flac", 4))
		file_type = FILE_TYPE_FLAC;
	else if (!strncasecmp(FS_GetFileExt(path), "mp3", 3))
		file_type = FILE_TYPE_MP3;
	else if (!strncasecmp(FS_GetFileExt(path), "ogg", 3))
		file_type = FILE_TYPE_OGG;
	else if (!strncasecmp(FS_GetFileExt(path), "opus", 4))
		file_type = FILE_TYPE_OPUS;
	else if (!strncasecmp(FS_GetFileExt(path), "wav", 3))
		file_type = FILE_TYPE_WAV;
	else if ((!strncasecmp(FS_GetFileExt(path), "it", 2)) || (!strncasecmp(FS_GetFileExt(path), "mod", 3))
		|| (!strncasecmp(FS_GetFileExt(path), "s3m", 3)) || (!strncasecmp(FS_GetFileExt(path), "xm", 2)))
		file_type = FILE_TYPE_XM;

	switch(file_type) {
		case FILE_TYPE_FLAC:
			decoder.init = FLAC_Init;
			decoder.rate = FLAC_GetSampleRate;
			decoder.channels = FLAC_GetChannels;
			decoder.decode = FLAC_Decode;
			decoder.position = FLAC_GetPosition;
			decoder.length = FLAC_GetLength;
			decoder.seek = FLAC_Seek;
			decoder.term = FLAC_Term;
			break;

		case FILE_TYPE_MP3:
			decoder.init = MP3_Init;
			decoder.rate = MP3_GetSampleRate;
			decoder.channels = MP3_GetChannels;
			decoder.decode = MP3_Decode;
			decoder.position = MP3_GetPosition;
			decoder.length = MP3_GetLength;
			decoder.seek = MP3_Seek;
			decoder.term = MP3_Term;
			break;

		case FILE_TYPE_OGG:
			decoder.init = OGG_Init;
			decoder.rate = OGG_GetSampleRate;
			decoder.channels = OGG_GetChannels;
			decoder.decode = OGG_Decode;
			decoder.position = OGG_GetPosition;
			decoder.length = OGG_GetLength;
			decoder.seek = OGG_Seek;
			decoder.term = OGG_Term;
			break;

		case FILE_TYPE_OPUS:
			decoder.init = OPUS_Init;
			decoder.rate = OPUS_GetSampleRate;
			decoder.channels = OPUS_GetChannels;
			decoder.decode = OPUS_Decode;
			decoder.position = OPUS_GetPosition;
			decoder.length = OPUS_GetLength;
			decoder.seek = OPUS_Seek;
			decoder.term = OPUS_Term;
			break;

		case FILE_TYPE_WAV:
			decoder.init = WAV_Init;
			decoder.rate = WAV_GetSampleRate;
			decoder.channels = WAV_GetChannels;
			decoder.decode = WAV_Decode;
			decoder.position = WAV_GetPosition;
			decoder.length = WAV_GetLength;
			decoder.seek = WAV_Seek;
			decoder.term = WAV_Term;
			break;

		case FILE_TYPE_XM:
			decoder.init = XM_Init;
			decoder.rate = XM_GetSampleRate;
			decoder.channels = XM_GetChannels;
			decoder.decode = XM_Decode;
			decoder.position = XM_GetPosition;
			decoder.length = XM_GetLength;
			decoder.seek = XM_Seek;
			decoder.term = XM_Term;
			break;

		default:
			break;
	}

	(* decoder.init)(path);
	pspAudioInit((* decoder.channels)() == 2? PSP_AUDIO_FORMAT_STEREO : PSP_AUDIO_FORMAT_MONO);
	pspAudioSetFrequency((* decoder.rate)() == 48000? 48000 : 44100);
	pspAudioSetChannelCallback(0, Audio_Decode, NULL);
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
	return (* decoder.position)();
}

u64 Audio_GetLength(void) {
	return (* decoder.length)();
}

u64 Audio_GetPositionSeconds(void) {
	return (Audio_GetPosition() / (* decoder.rate)());
}

u64 Audio_GetLengthSeconds(void) {
	return (Audio_GetLength() / (* decoder.rate)());
}

void Audio_Term(void) {
	playing = true;
	paused = false;

	pspAudioSetChannelCallback(0, NULL, NULL); // Clear channel callback
	pspAudioEndPre();
	sceKernelDelayThread(50 * 1000);
	pspAudioEnd();
	(* decoder.term)();

	// Clear metadata struct
	metadata = empty_metadata;
	decoder = empty_decoder;
}
