#include "audio.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include "fs.h"

static drwav wav;
static drwav_uint64 samples_read = 0;

int WAV_Init(const char *path) {
	if (!drwav_init_file(&wav, path))
		return -1;

	return 0;
}

void WAV_Decode(void *buf, unsigned int length, void *userdata) {
	samples_read += drwav_read_pcm_frames_s16(&wav, (drwav_uint64)length, (drwav_int16 *)buf);

	if (samples_read == wav.totalPCMFrameCount)
		playing = false;
}

u64 WAV_GetPosition(void) {
	return samples_read;
}

u64 WAV_GetLength(void) {
	return wav.totalPCMFrameCount;
}

u64 WAV_GetPositionSeconds(const char *path) {
	return (samples_read / wav.sampleRate);
}

u64 WAV_GetLengthSeconds(const char *path) {
	return (wav.totalPCMFrameCount / wav.sampleRate);
}

void WAV_Term(void) {
	samples_read = 0;drwav_uninit(&wav);
}
