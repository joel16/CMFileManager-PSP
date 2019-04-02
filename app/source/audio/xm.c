#include "audio.h"
#include "xmp.h"

static xmp_context xmp;
static struct xmp_frame_info frame_info;
static SceUInt64 samples_read = 0, total_samples = 0;

int XM_Init(const char *path) {
    xmp = xmp_create_context();
    if (xmp_load_module(xmp, (char *)path) < 0)
        return -1;

    xmp_start_player(xmp, 44100, 0);
    xmp_get_frame_info(xmp, &frame_info);
    total_samples = (frame_info.total_time * 44.1);
    return 0;
}

u32 XM_GetSampleRate(void) {
    return 44100;
}

u8 XM_GetChannels(void) {
	return 2;
}

void XM_Decode(void *buf, unsigned int length, void *userdata) {
    xmp_play_buffer(xmp, buf, length * (sizeof(s16) * 2), 0);
    samples_read += length;

    if (samples_read == total_samples)
        playing = false;
}

u64 XM_GetPosition(void) {
    return samples_read;
}

u64 XM_GetLength(void) {
    return total_samples;
}

void XM_Term(void) {
    samples_read = 0;
    xmp_end_player(xmp);
    xmp_release_module(xmp);
    xmp_free_context(xmp);
}
