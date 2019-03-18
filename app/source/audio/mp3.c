#include "audio.h"

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

static drmp3 mp3;
static drmp3_uint64 frames_read = 0;
static drmp3_uint64 frame_count = 0;

int MP3_Init(const char *path) {
	if (!drmp3_init_file(&mp3, path, NULL))
		return -1;

	frame_count = drmp3_get_pcm_frame_count(&mp3);
	
	return 0;
}

void MP3_Decode(void *buf, unsigned int length, void *userdata) {
	float audio_buf[frame_count];
	frames_read += drmp3_read_pcm_frames_f32(&mp3, length, audio_buf);
	drmp3dec_f32_to_s16(audio_buf, (drmp3_int16 *)buf, length * 2);

	if (frames_read == frame_count)
		playing = false;
}

u64 MP3_GetPosition(void) {
	return frames_read;
}

u64 MP3_GetLength(void) {
	return frame_count;
}

u64 MP3_GetLengthSeconds(const char *path) {
	return (frame_count / mp3.sampleRate);
}

void MP3_Term(void) {
	frames_read = 0;
	drmp3_uninit(&mp3);
}
