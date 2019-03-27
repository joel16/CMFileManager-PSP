#include "audio.h"
#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

static drflac *flac;
static drflac_uint64 frames_read = 0;

int FLAC_Init(const char *path) {
	flac = drflac_open_file(path);
	if (flac == NULL)
		return -1;

	return 0;
}

void FLAC_Decode(void *buf, unsigned int length, void *userdata) {
	frames_read += drflac_read_pcm_frames_s16(flac, (drflac_uint64)length, (drflac_int16 *)buf);
	
	if (frames_read == flac->totalPCMFrameCount)
		playing = false;
}

u64 FLAC_GetPosition(void) {
	return frames_read;
}

u64 FLAC_GetLength(void) {
	return flac->totalPCMFrameCount;
}

u64 FLAC_GetPositionSeconds(const char *path) {
	return (frames_read / flac->sampleRate);
}

u64 FLAC_GetLengthSeconds(const char *path) {
	return (flac->totalPCMFrameCount / flac->sampleRate);
}

void FLAC_Term(void) {
	frames_read = 0;
	drflac_close(flac);
}
