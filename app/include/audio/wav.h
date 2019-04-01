#pragma once

int WAV_Init(const char *path);
u32 WAV_GetSampleRate(void);
void WAV_Decode(void *buf, unsigned int length, void *userdata);
u64 WAV_GetPosition(void);
u64 WAV_GetLength(void);
void WAV_Term(void);
