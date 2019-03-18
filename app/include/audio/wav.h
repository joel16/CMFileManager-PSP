#pragma once

int WAV_Init(const char *path);
void WAV_Decode(void *buf, unsigned int length, void *userdata);
u64 WAV_GetPosition(void);
u64 WAV_GetLength(void);
u64 WAV_GetPositionSeconds(const char *path);
u64 WAV_GetLengthSeconds(const char *path);
void WAV_Term(void);
