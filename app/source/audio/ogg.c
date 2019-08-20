#include <pspiofilemgr.h>

#include "audio/audio.h"
#include "utils.h"
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

static OggVorbis_File ogg;
static SceUID ogg_file = 0;
static vorbis_info *ogg_info = NULL;
static ogg_int64_t samples_read = 0, max_lenth = 0;

size_t ogg_callback_read(void *ptr, size_t size, size_t count, void *stream) {
	return sceIoRead(*(SceUID *)stream, ptr, size * count);
}

int ogg_callback_seek(void *stream, ogg_int64_t offset, int whence) {
	return sceIoLseek32(*(SceUID *)stream, (unsigned int) offset, whence);
}

int ogg_callback_close(void *stream) {
	return sceIoClose(*(SceUID *)stream);
}

long ogg_callback_tell(void *stream) {
	return sceIoLseek32(*(SceUID *)stream, 0, PSP_SEEK_CUR);
}

int OGG_Init(const char *path) {
	if (R_FAILED(ogg_file = sceIoOpen(path, PSP_O_RDONLY, 0777)))
		return -1;

	ov_callbacks ogg_callbacks;
	ogg_callbacks.read_func = ogg_callback_read;
	ogg_callbacks.seek_func = ogg_callback_seek;
	ogg_callbacks.close_func = ogg_callback_close;
	ogg_callbacks.tell_func = ogg_callback_tell;

	if (R_FAILED(ov_open_callbacks(&ogg_file, &ogg, NULL, 0, ogg_callbacks))) {
		sceIoClose(ogg_file);
		return -1;
	}

	if ((ogg_info = ov_info(&ogg, -1)) == NULL)
		return -1;

	max_lenth = ov_pcm_total(&ogg, -1);

	vorbis_comment *comment = ov_comment(&ogg, -1);
	if (comment != NULL) {
		metadata.has_meta = true;

		char *value = NULL;

		if ((value = vorbis_comment_query(comment, "title", 0)) != NULL)
			snprintf(metadata.title, 31, "%s\n", value);

		if ((value = vorbis_comment_query(comment, "album", 0)) != NULL)
			snprintf(metadata.album, 31, "%s\n", value);

		if ((value = vorbis_comment_query(comment, "artist", 0)) != NULL)
			snprintf(metadata.artist, 31, "%s\n", value);

		if ((value = vorbis_comment_query(comment, "year", 0)) != NULL)
			snprintf(metadata.year, 5, "%s\n", value);

		if ((value = vorbis_comment_query(comment, "comment", 0)) != NULL)
			snprintf(metadata.comment, 31, "%s\n", value);

		if ((value = vorbis_comment_query(comment, "genre", 0)) != NULL)
			snprintf(metadata.genre, 31, "%s\n", value);
	}

	return 0;
}

u32 OGG_GetSampleRate(void) {
	return ogg_info->rate;
}

u8 OGG_GetChannels(void) {
	return ogg_info->channels;
}

static u64 OGG_FillBuffer(char *out) {
	u64 samples_read = 0;
	int samples_to_read = (sizeof(s16) * ogg_info->channels) * 960;

	while(samples_to_read > 0) {
		static int current_section;
		int samples_just_read = ov_read(&ogg, out, samples_to_read > 960 ? 960 : samples_to_read, 0, 2, 1, &current_section);

		if (samples_just_read < 0)
			return samples_just_read;
		else if (samples_just_read == 0)
			break;

		samples_read += samples_just_read;
		samples_to_read -= samples_just_read;
		out += samples_just_read;
	}

	return samples_read / sizeof(s16);
}

void OGG_Decode(void *buf, unsigned int length, void *userdata) {
	OGG_FillBuffer((char *)buf);
	samples_read = ov_pcm_tell(&ogg);

	if (samples_read >= max_lenth)
		playing = false;
}

u64 OGG_GetPosition(void) {
	return samples_read;
}

u64 OGG_GetLength(void) {
	return max_lenth;
}

u64 OGG_Seek(u64 index) {
	ogg_int64_t seek_sample = (max_lenth * (index / 225.0));
	
	if (ov_pcm_seek(&ogg, seek_sample) >= 0) {
		samples_read = seek_sample;
		return samples_read;
	}

	return -1;
}

void OGG_Term(void) {
	samples_read = 0;

	if (metadata.has_meta)
        metadata.has_meta = false;

	ov_clear(&ogg);
	sceIoClose(ogg_file);
}
