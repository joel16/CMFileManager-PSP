#pragma once

/// Checks whether a result code indicates success.
#define R_SUCCEEDED(res)   ((res)>=0)
/// Checks whether a result code indicates failure.
#define R_FAILED(res)      ((res)<0)

extern bool psp_usb_cable_connection;
int OSL_KEYMASK_ENTER, OSL_KEYMASK_CANCEL;

void Utils_SetMax(int *set, int value, int max);
void Utils_SetMin(int *set, int value, int min);
char *Utils_Basename(const char *filename);
void Utils_GetSizeString(char *string, u64 size);
void Utils_AppendArr(char subject[], const char insert[], int pos);
int Utils_Alphasort(const void *p1, const void *p2);
void Utils_HandleUSB(void);
bool Utils_IsEF0(void);
int Utils_LaunchEboot(const char *path);
int Utils_LaunchPOPS(const char *path);
int Utils_LaunchISO(const char *path);
u64 Utils_GetTotalStorage(void);
u64 Utils_GetUsedStorage(void);
int Utils_GetEnterButton(void);
int Utils_GetCancelButton(void);
