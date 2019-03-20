#pragma once

int OGG_Init(const char *path);
void OGG_Decode(void *buf, unsigned int length, void *userdata);
u64 OGG_GetPosition(void);
u64 OGG_GetLength(void);
u64 OGG_GetPositionSeconds(const char *path);
u64 OGG_GetLengthSeconds(const char *path);
void OGG_Term(void);
