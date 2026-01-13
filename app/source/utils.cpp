#include <cstdio>
#include <cstring>
#include <pspreg.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include <psputility_sysparam.h>
#include <vector>

#include "config.h"
#include "kernel_functions.h"
#include "kubridge.h"
#include "log.h"
#include "pspusbdevice.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "utils.h"

bool isMSInserted = false, isPSPGo = false;
enum PspCtrlButtons PSP_CTRL_ENTER, PSP_CTRL_CANCEL;
BROWSE_STATE device = BROWSE_STATE_EXTERNAL;
int language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;

namespace Utils {
    constexpr unsigned int CTRL_DEADZONE_DELAY = 500000;
    constexpr unsigned int CTRL_DELAY = 100000;

    static SceCtrlData pad, prevPad;
    static unsigned int lastButton = 0;
    static int lastButtonTick = 0, deadzoneTick = 0;
    static bool usbModuleLoaded = false, usbActivated = false, usbConnected = false;
    
    struct Module {
        const char* path;
        SceUID id;
    };
    
    static std::vector<Module> kernelModules {
        { "audio_driver.prx", -1 },
        { "display_driver.prx", -1 },
        { "fs_driver.prx", -1 }
    };
    
    static std::vector<Module> usbModules {
        { "flash0:/kd/semawm.prx", -1 },
        { "flash0:/kd/usbstor.prx", -1 },
        { "flash0:/kd/usbstormgr.prx", -1 },
        { "flash0:/kd/usbstorms.prx", -1 },
        { "flash0:/kd/usbstoreflash.prx", -1 },
        { "flash0:/kd/usbstorboot.prx", -1 }
    };
    
    typedef struct {
        unsigned long maxClusters = 0;
        unsigned long freeClusters = 0;
        int unk1 = 0;
        unsigned int sectorSize = 0;
        u64 sectorCount = 0;
    } SystemDevCtl;
    
    typedef struct {
        SystemDevCtl *devCtl;    
    } SystemDevCommand;
    
    void SetBounds(int &set, int min, int max) {
        if (set > max) {
            set = min;
        }
        else if (set < min) {
            set = max;
        }
    }

    void SetMax(int &set, int value, int max) {
        if (set > max) {
            set = value;
        }
    }

    void SetMin(int &set, int value, int min) {
        if (set < min) {
            set = value;
        }
    }

    void GetSizeString(char *string, double size) {
        int i = 0;
        const char *units[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
        
        while (size >= 1024.f) {
            size /= 1024.f;
            i++;
        }
        
        std::sprintf(string, "%.*f %s", (i == 0) ? 0 : 2, size, units[i]);
    }

    static int LoadStartModule(const char *path) {
        int ret = 0, status = 0;
        SceUID modID = 0;
        
        if (R_FAILED(ret = modID = kuKernelLoadModule(path, 0, nullptr))) {
            Log::Error("kuKernelLoadModule(%s) failed: 0x%08x\n", path, ret);
            return ret;
        }
        
        if (R_FAILED(ret = sceKernelStartModule(modID, 0, nullptr, &status, nullptr))) {
            Log::Error("sceKernelStartModule(%s) failed: 0x%08x\n", path, ret);
            return ret;
        }
        
        return ret;
    }

    static void StopUnloadModules(SceUID modID) {
        sceKernelStopModule(modID, 0, nullptr, nullptr, nullptr);
        sceKernelUnloadModule(modID);
    }

    static int InitUSB(void) {
        int ret = 0;
        
        if (!usbModuleLoaded) {
            for (unsigned int i = 0; i < usbModules.size(); ++i) {
                usbModules[i].id = Utils::LoadStartModule(usbModules[i].path);
            }
                
            usbModuleLoaded = true;
        }
        
        if (R_FAILED(ret = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0))) {
            Log::Error("sceUsbStart(PSP_USBBUS_DRIVERNAME) failed: 0x%08x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0))) {
            Log::Error("sceUsbStart(PSP_USBSTOR_DRIVERNAME) failed: 0x%08x\n", ret);
            return ret;
        }
        
        usbActivated = true;
        return 0;
    }

    static int StartUSBStorage(void) {
        int ret = 0;
        
        if (R_FAILED(ret = sceUsbActivate(0x1c8))) {
            Log::Error("sceUsbActivate(0x1c8) failed: 0x%08x\n", ret);
            return ret;
        }
        
        usbConnected = true;
        return 0;
    }

    static int StopUSBStorage(void) {
        int ret = 0;
        
        if (R_FAILED(ret = sceUsbDeactivate(0x1c8))) {
            Log::Error("sceUsbActivate(0x1c8) failed: 0x%08x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = sceIoDevctl("fatms0:", 0x0240D81E, nullptr, 0, nullptr, 0))) { // Avoid corrupted files
            Log::Error("sceIoDevctl(\"fatms0:\", 0x0240D81E, nullptr, 0, nullptr, 0) failed: 0x%08x\n", ret);
            return ret;
        }
        
        usbConnected = false;
        return 0;
    }

    static int DisableUSB(void) {
        int ret = 0;
        
        if (!usbActivated) {
            return -1;
        }
            
        if (R_FAILED(ret = Utils::StopUSBStorage())) {
            return ret;
        }
            
        if (R_FAILED(ret = sceUsbStop(PSP_USBSTOR_DRIVERNAME, 0, 0))) {
            Log::Error("sceUsbStop(PSP_USBSTOR_DRIVERNAME) failed: 0x%08x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0))) {
            Log::Error("sceUsbStop(PSP_USBBUS_DRIVERNAME) failed: 0x%08x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = pspUsbDeviceFinishDevice())) {
            Log::Error("pspUsbDeviceFinishDevice() failed: 0x%08x\n", ret);
            return ret;
        }
        
        usbActivated = false;
        return 0;
    }

    static void ExitUSB(void) {
        Utils::DisableUSB();
        
        if (usbModuleLoaded) {
            for (int i = usbModules.size() - 1; i >= 0; --i) {
                Utils::StopUnloadModules(usbModules[i].id);
                usbModules[i].id = -1;
            }
            
            usbModuleLoaded = false;
        }
    }

    void InitKernelDrivers(void) {
        for (unsigned int i = 0; i < kernelModules.size(); ++i) {
            kernelModules[i].id = Utils::LoadStartModule(kernelModules[i].path);
        }
        
        Utils::InitUSB();
    }

    void TermKernelDrivers(void) {
        for (int i = kernelModules.size() - 1; i >= 0; --i) {
            Utils::StopUnloadModules(kernelModules[i].id);
            kernelModules[i].id = -1;
        }
        
        Utils::ExitUSB();
    }
    
    void UpdateUSB(void) {
        if (sceUsbGetState() & PSP_USB_CABLE_CONNECTED) {
            if (usbConnected == false) {
                Utils::StartUSBStorage();
            }
        }
        else {
            if (usbConnected == true) {
                Utils::StopUSBStorage();
            }
        }
    }

    bool IsModelPSPGo(void) {
        return (kuKernelGetModel() == 4);
    }

    int IsMemCardInserted(bool &isInserted) {
        int status = 0, ret = 0;
        if (R_FAILED(ret = sceIoDevctl("mscmhc0:", 0x02025806, 0, 0, &status, sizeof(status)))) {
            return ret;
        }
            
        if (status != 1) {
            isInserted = false;
        }
        else {
            isInserted = true;
        }
        
        return 0;
    }

    bool IsInternalStorage(void) {
        if (isPSPGo) {
            if (!isMSInserted) {
                return true;
            }
                
            return true;
        }
        
        return false;
    }

    int LaunchEboot(const char *path) {
        int ret = 0;
        struct SceKernelLoadExecVSHParam param;
        std::memset(&param, 0, sizeof(param));
        
        param.size = sizeof(param);
        param.args = std::strlen(path) + 1;
        param.argp = (void *)path;
        param.key = "game";
        
        if (R_FAILED(ret = sctrlKernelLoadExecVSHWithApitype(Utils::IsInternalStorage()? 0x152 : PSP_INIT_APITYPE_MS2, path, &param))) {
            Log::Error("sctrlKernelLoadExecVSHWithApitype(%x, %s) failed: 0x%08x\n", Utils::IsInternalStorage()? 0x152 : PSP_INIT_APITYPE_MS2, path, ret);
            return ret;
        }
        
        return 0;
    }

    u64 GetTotalStorage(void) {
        int ret = 0;
        SystemDevCtl devctl;
        SystemDevCommand command;
        command.devCtl = &devctl;
        
        if (R_FAILED(ret = sceIoDevctl(device == BROWSE_STATE_INTERNAL? "ef0": "ms0:", 0x02425818, &command, sizeof(SystemDevCommand), nullptr, 0))) {
            return 0;
        }
            
        u64 size = (devctl.maxClusters * devctl.sectorCount) * devctl.sectorSize;
        return size;
    }

    u64 GetFreeStorage(void) {
        int ret = 0;
        SystemDevCtl devctl;
        SystemDevCommand command;
        command.devCtl = &devctl;
        
        if (R_FAILED(ret = sceIoDevctl(device == BROWSE_STATE_INTERNAL? "ef0": "ms0:", 0x02425818, &command, sizeof(SystemDevCommand), nullptr, 0))) {
            return 0;
        }
            
        u64 size = (devctl.freeClusters * devctl.sectorCount) * devctl.sectorSize; 
        return size;
    }

    u64 GetUsedStorage(void) {
        return (Utils::GetTotalStorage() - Utils::GetFreeStorage());
    }
    
    static int GetRegistryValue(const char *dir, const char *name, unsigned int *value) {
        int ret = 0;
        struct RegParam regParam;
        REGHANDLE regHandle = 0, regHandleCategory = 0, regHandleKey = 0;
        unsigned int type = 0, size = 0;
        
        std::memset(&regParam, 0, sizeof(RegParam));
        regParam.regtype = 1;
        regParam.namelen = std::strlen("/system");
        regParam.unk2 = 1;
        regParam.unk3 = 1;
        std::strcpy(regParam.name, "/system");

        if (R_FAILED(ret = sceRegOpenRegistry(&regParam, 2, &regHandle))) {
            Log::Error("sceRegOpenRegistry() failed: 0x%08x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = sceRegOpenCategory(regHandle, dir, 2, &regHandleCategory))) {
            sceRegCloseRegistry(regHandle);
            Log::Error("sceRegOpenCategory() failed: 0x%08x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = sceRegGetKeyInfo(regHandleCategory, name, &regHandleKey, &type, &size))) {
            sceRegCloseCategory(regHandleCategory);
            sceRegCloseRegistry(regHandle);
            Log::Error("sceRegGetKeyInfo() failed: 0x%08x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = sceRegGetKeyValue(regHandleCategory, regHandleKey, value, 4))) {
            sceRegCloseCategory(regHandleCategory);
            sceRegCloseRegistry(regHandle);
            Log::Error("sceRegGetKeyValue() failed: 0x%08x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = sceRegFlushCategory(regHandleCategory))) {
            sceRegCloseCategory(regHandleCategory);
            sceRegCloseRegistry(regHandle);
            Log::Error("sceRegFlushCategory() failed: 0x%08x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = sceRegCloseCategory(regHandleCategory))) {
            sceRegCloseRegistry(regHandle);
            Log::Error("sceRegCloseCategory() failed: 0x%08x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = sceRegFlushRegistry(regHandle))) {
            sceRegCloseRegistry(regHandle);
            Log::Error("sceRegFlushRegistry() failed: 0x%08x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = sceRegCloseRegistry(regHandle))) {
            Log::Error("sceRegFlushRegistry() failed: 0x%08x\n", ret);
            return ret;
        }
            
        return 0;
    }
    
    int ReadControls(void) {
        prevPad = pad;
        sceCtrlReadBufferPositive(&pad, 1);
        
        if (pad.Buttons == lastButton) {
            if (pad.TimeStamp - deadzoneTick < CTRL_DEADZONE_DELAY) {
                return 0;
            }
                
            if (pad.TimeStamp - lastButtonTick < CTRL_DELAY) {
                return 0;
            }
                
            lastButtonTick = pad.TimeStamp;
            return lastButton;
        }
        
        lastButton = pad.Buttons;
        deadzoneTick = lastButtonTick = pad.TimeStamp;
        return lastButton;
    }
    
    int IsButtonPressed(enum PspCtrlButtons buttons) {
        return ((pad.Buttons & buttons) == buttons) && ((prevPad.Buttons & buttons) != buttons);
    }
    
    int IsButtonHeld(enum PspCtrlButtons buttons) {
        return pad.Buttons & buttons;
    }
    
    static enum PspCtrlButtons GetAssignedButton(bool enter) {
        int ret = 0, button = -1;
        
        if (R_FAILED(ret = sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &button))) {
            Log::Error("sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN) failed: 0x%08x\n", ret);
    
            unsigned int regButton = -1;
            if (R_SUCCEEDED(Utils::GetRegistryValue("/CONFIG/SYSTEM/XMB", "button_assign", &regButton))) {
                return (regButton == 0) == enter ? PSP_CTRL_CIRCLE : PSP_CTRL_CROSS;
            }

            return enter ? PSP_CTRL_CROSS : PSP_CTRL_CIRCLE;
        }
        
        return (button == 0) == enter ? PSP_CTRL_CIRCLE : PSP_CTRL_CROSS;
    }
    
    enum PspCtrlButtons GetEnterButton(void) {
        return Utils::GetAssignedButton(true);
    }
    
    enum PspCtrlButtons GetCancelButton(void) {
        return Utils::GetAssignedButton(false);
    }    
    
    float GetAnalogX(void) {
        return ((static_cast<float>(pad.Lx - 122.5f)) / 122.5f);
    }
    
    float GetAnalogY(void) {
        return ((static_cast<float>(pad.Ly - 122.5f)) / 122.5f);
    }
    
    bool IsCancelButtonPressed(void) {
        Utils::ReadControls();
        
        if (Utils::IsButtonPressed(PSP_CTRL_CANCEL)) {
            return true;
        }
            
        return false;
    }

    int GetLanguage(void) {
        int ret = 0;
        int language = 0;

        if (R_FAILED(ret = sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &language))) {
            Log::Error("sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE) failed: 0x%08x\n", ret);
            return PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
        }

        return language;
    }
}
