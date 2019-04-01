#pragma once

int XM_Init(const char *path);
u32 XM_GetSampleRate(void);
void XM_Decode(void *buf, unsigned int length, void *userdata);
u64 XM_GetPosition(void);
u64 XM_GetLength(void);
void XM_Term(void);
