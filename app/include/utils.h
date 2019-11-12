#pragma once

#include <pspctrl.h>
#include <pspmodulemgr.h>
#include <stdbool.h>

/// Checks whether a result code indicates success.
#define R_SUCCEEDED(res)   ((res) >= 0)
/// Checks whether a result code indicates failure.
#define R_FAILED(res)      ((res) < 0)

#define PSP_GO 4

extern bool psp_usb_cable_connection;
enum PspCtrlButtons PSP_CTRL_ENTER, PSP_CTRL_CANCEL;

void Utils_SetMax(int *set, int value, int max);
void Utils_SetMin(int *set, int value, int min);
char *Utils_Basename(const char *filename);
void Utils_GetSizeString(char *string, u64 size);
void Utils_AppendArr(char subject[], const char insert[], int pos);
int Utils_Alphasort(const void *p1, const void *p2);
int Utils_InitAudioDriver(void);
void Utils_ExitAudioDriver(void);
int Utils_InitDisplayDriver(void);
void Utils_ExitDisplayDriver(void);
int Utils_GetAudioDriverInfo(SceKernelModuleInfo *info);
int Utils_GetDisplayDriverInfo(SceKernelModuleInfo *info);
int Utils_InitUSB(void);
void Utils_ExitUSB(void);
void Utils_HandleUSB(void);
bool Utils_IsModelPSPGo(void);
int Utils_IsMemCardInserted(bool *is_inserted);
bool Utils_IsEF0(void);
int Utils_LaunchEboot(const char *path);
u64 Utils_GetTotalStorage(void);
u64 Utils_GetUsedStorage(void);
void Utils_ReadControls(void);
int Utils_IsButtonPressed(enum PspCtrlButtons buttons);
int Utils_IsButtonHeld(enum PspCtrlButtons buttons);
int Utils_GetEnterButton(void);
int Utils_GetCancelButton(void);
float Utils_GetAnalogX(void);
float Utils_GetAnalogY(void);
