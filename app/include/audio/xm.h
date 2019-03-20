#pragma once

int XM_Init(const char *path);
void XM_Decode(void *buf, unsigned int length, void *userdata);
u64 XM_GetPosition(void);
u64 XM_GetLength(void);
u64 XM_GetPositionSeconds(const char *path);
u64 XM_GetLengthSeconds(const char *path);
void XM_Term(void);
