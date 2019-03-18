#pragma once

#include <psptypes.h>
#include <stdbool.h>

extern bool playing, paused;

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
