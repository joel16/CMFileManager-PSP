#pragma once

int MP3_Init(const char *path);
void MP3_Decode(void *buf, unsigned int length, void *userdata);
u64 MP3_GetPosition(void);
u64 MP3_GetLength(void);
u64 MP3_GetPositionSeconds(const char *path);
u64 MP3_GetLengthSeconds(const char *path);
void MP3_Term(void);
