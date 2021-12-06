#ifndef _CMFILEMANAGER_UTILS_H_
#define _CMFILEMANAGER_UTILS_H_

#include <pspctrl.h>
#include <psptypes.h>

/// Checks whether a result code indicates success.
#define R_SUCCEEDED(res) ((res) >= 0)
/// Checks whether a result code indicates failure.
#define R_FAILED(res)    ((res) < 0)

enum BROWSE_STATE {
    BROWSE_STATE_INTERNAL,
    BROWSE_STATE_EXTERNAL,
    BROWSE_STATE_FLASH0,
    BROWSE_STATE_FLASH1,
    BROWSE_STATE_FLASH2,
    BROWSE_STATE_FLASH3,
    BROWSE_STATE_UMD
};

extern bool psp_usb_cable_connection, is_ms_inserted, is_psp_go;
extern enum PspCtrlButtons PSP_CTRL_ENTER, PSP_CTRL_CANCEL;
extern BROWSE_STATE device;

namespace Utils {
    void SetBounds(int &set, int min, int max);
    void SetMax(int &set, int value, int max);
    void SetMin(int &set, int value, int min);
    void GetSizeString(char *string, double size);
    void InitKernelDrivers(void);
    void TermKernelDrivers(void);
    void HandleUSB(void);
    bool IsModelPSPGo(void);
    int IsMemCardInserted(bool &is_inserted);
    bool IsInternalStorage(void);
    int LaunchEboot(const char *path);
    u64 GetTotalStorage(void);
    u64 GetFreeStorage(void);
    u64 GetUsedStorage(void);
    int ReadControls(void);
    int IsButtonPressed(enum PspCtrlButtons buttons);
    int IsButtonHeld(enum PspCtrlButtons buttons);
    int IsKButtonPressed(enum PspCtrlButtons buttons);
    int IsKButtonHeld(enum PspCtrlButtons buttons);
    enum PspCtrlButtons GetEnterButton(void);
    enum PspCtrlButtons GetCancelButton(void);
    float GetAnalogX(void);
    float GetAnalogY(void);
    bool IsCancelButtonPressed(void);
}

#endif
