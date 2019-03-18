#include "audio.h"
#include "fs.h"
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

static stb_vorbis *ogg;
static stb_vorbis_info ogg_info;
static int samples_read = 0;

int OGG_Init(const char *path) {
	int error = 0;
	ogg = stb_vorbis_open_filename(path, &error, NULL);

	if (!ogg)
		return -1;

	return 0;
}

void OGG_Decode(void *buf, unsigned int length, void *userdata) {
	samples_read += stb_vorbis_get_samples_short_interleaved(ogg, 2, (short *)buf, length * 2);

	//if (samples_read == wav.totalPCMFrameCount)
	//	playing = false;
}

u64 OGG_GetPosition(void) {
	return samples_read;
}

u64 OGG_GetLength(void) {
	return 0;
}

u64 OGG_GetLengthSeconds(const char *path) {
	return 0;
}

void OGG_Term(void) {
	samples_read = 0;
	stb_vorbis_close(ogg);
}