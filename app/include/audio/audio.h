#pragma once

#include <glib2d.h>
#include <psptypes.h>
#include <stdbool.h>

extern bool playing, paused;

typedef struct {
	bool has_meta;
    char title[31];
    char album[31];
    char artist[31];
    char year[5];
    char comment[31];
    char genre[31];
    g2dTexture *cover_image;
} Audio_Metadata;

extern Audio_Metadata metadata;

void Audio_Init(const char *path);
void Audio_Decode(void *buf, unsigned int length, void *userdata);
bool Audio_IsPaused(void);
void Audio_Pause(void);
void Audio_Stop(void);
u64 Audio_GetPosition(void);
u64 Audio_GetLength(void);
u64 Audio_GetPositionSeconds(const char *path);
u64 Audio_GetLengthSeconds(const char *path);
void Audio_Term(void);
