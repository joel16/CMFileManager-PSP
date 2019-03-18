#pragma once

int FLAC_Init(const char *path);
void FLAC_Decode(void *buf, unsigned int length, void *userdata);
u64 FLAC_GetPosition(void);
u64 FLAC_GetLength(void);
u64 FLAC_GetPositionSeconds(const char *path);
u64 FLAC_GetLengthSeconds(const char *path);
void FLAC_Term(void);
