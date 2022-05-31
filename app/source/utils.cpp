#include <cstdio>
#include <cstring>
#include <pspkernel.h>
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

bool psp_usb_cable_connection = false, is_ms_inserted = false, is_psp_go = false;
enum PspCtrlButtons PSP_CTRL_ENTER, PSP_CTRL_CANCEL;
BROWSE_STATE device = BROWSE_STATE_EXTERNAL;
int g_psp_language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;

extern unsigned char audio_driver_prx_start[], display_driver_prx_start[], fs_driver_prx_start[];
extern unsigned int audio_driver_prx_size, display_driver_prx_size, fs_driver_prx_size;

namespace Utils {
    constexpr unsigned int CTRL_DEADZONE_DELAY = 500000;
    constexpr unsigned int CTRL_DELAY = 100000;

    static SceCtrlData pad, prev_pad;
    static unsigned int last_button = 0;
    static int last_button_tick = 0, deadzone_tick = 0;
    static bool usb_module_loaded = false;
    static bool usb_actived = false;
    
    typedef struct {
        const char *path = nullptr;
        int id = 0;
        unsigned char *data = nullptr;
        unsigned int size = 0;
    } Module;
    
    static std::vector<Module> kernel_modules {
        { "audio_driver.prx", -1, audio_driver_prx_start, audio_driver_prx_size },
        { "display_driver.prx", -1, display_driver_prx_start, display_driver_prx_size },
        { "fs_driver.prx", -1, fs_driver_prx_start, fs_driver_prx_size }
    };
    
    static std::vector<Module> usb_modules {
        { "flash0:/kd/_usbdevice.prx", -1, },
        { "flash0:/kd/semawm.prx", -1, },
        { "flash0:/kd/usbstor.prx", -1, },
        { "flash0:/kd/usbstormgr.prx", -1, },
        { "flash0:/kd/usbstorms.prx", -1, },
        { "flash0:/kd/usbstoreflash.prx", -1, },
        { "flash0:/kd/usbstorboot.prx", -1, }
    };
    
    typedef struct {
        unsigned long maxclusters = 0;
        unsigned long freeclusters = 0;
        int unk1 = 0;
        unsigned int sectorsize = 0;
        u64 sectorcount = 0;
    } SystemDevCtl;
    
    typedef struct {
        SystemDevCtl *pdevinf;    
    } SystemDevCommand;
    
    void SetBounds(int &set, int min, int max) {
        if (set > max)
            set = min;
        else if (set < min)
            set = max;
    }

    void SetMax(int &set, int value, int max) {
        if (set > max)
            set = value;
    }

    void SetMin(int &set, int value, int min) {
        if (set < min)
            set = value;
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

    // Basically removes and re-creates prx from memory -> then remove it after inital load
    static int LoadStartModuleMem(const char *path, const void *buf, SceSize size) {
        int ret = 0;
        SceUID modID = 0;
        
        // Don't care if this passes or fails
        sceIoRemove(path);
        SceUID file = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT, 0777);
        sceIoWrite(file, buf, size);
        sceIoClose(file);
        
        if (R_FAILED(ret = modID = kuKernelLoadModule(path, 0, nullptr))) {
            Log::Error("kuKernelLoadModule(%s) failed: 0x%08x\n", path, ret);
            return ret;
        }
        
        if (R_FAILED(ret = sceKernelStartModule(modID, 0, nullptr, nullptr, nullptr))) {
            Log::Error("sceKernelStartModule(%s) failed: 0x%08x\n", path, ret);
            return ret;
        }
        
        sceIoRemove(path);
        return 0;
    }

    static void StopUnloadModules(SceUID modID) {
        sceKernelStopModule(modID, 0, nullptr, nullptr, nullptr);
        sceKernelUnloadModule(modID);
    }

    static int InitUSB(void) {
        int ret = 0;
        
        if (!usb_module_loaded) {
            for (unsigned int i = 0; i < usb_modules.size(); ++i)
                usb_modules[i].id = Utils::LoadStartModule(usb_modules[i].path);
                
            usb_module_loaded = true;
        }
        
        if (R_FAILED(ret = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0))) {
            Log::Error("sceUsbStart(PSP_USBBUS_DRIVERNAME) failed: 0x%08x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0))) {
            Log::Error("sceUsbStart(PSP_USBSTOR_DRIVERNAME) failed: 0x%08x\n", ret);
            return ret;
        }
        
        if (R_FAILED(ret = sceUsbstorBootSetCapacity(0x800000))) {
            Log::Error("sceUsbstorBootSetCapacity(0x800000) failed: 0x%08x\n", ret);
            return ret;
        }
        
        usb_actived = true;
        return 0;
    }

    static int StartUSBStorage(void) {
        int ret = 0;
        
        if (R_FAILED(ret = sceUsbActivate(0x1c8))) {
            Log::Error("sceUsbActivate(0x1c8) failed: 0x%08x\n", ret);
            return ret;
        }
        
        psp_usb_cable_connection = true;
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
        
        psp_usb_cable_connection = false;
        return 0;
    }

    static int DisableUSB(void) {
        int ret = 0;
        
        if (!usb_actived)
            return -1;
            
        if (R_FAILED(ret = Utils::StopUSBStorage()))
            return ret;
            
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
        
        usb_actived = false;
        return 0;
    }

    static void ExitUSB(void) {
        Utils::DisableUSB();
        
        if (usb_module_loaded) {
            for (int i = usb_modules.size() - 1; i >= 0; --i) {
                Utils::StopUnloadModules(usb_modules[i].id);
                usb_modules[i].id = -1;
            }
            
            usb_module_loaded = false;
        }
    }

    void InitKernelDrivers(void) {
        for (unsigned int i = 0; i < kernel_modules.size(); ++i)
            kernel_modules[i].id = Utils::LoadStartModuleMem(kernel_modules[i].path, kernel_modules[i].data, kernel_modules[i].size);
        
        Utils::InitUSB();
    }

    void TermKernelDrivers(void) {
        for (int i = kernel_modules.size() - 1; i >= 0; --i) {
            Utils::StopUnloadModules(kernel_modules[i].id);
            kernel_modules[i].id = -1;
        }
        
        Utils::ExitUSB();
    }
    
    void UpdateUSB(void) {
        if (sceUsbGetState() & PSP_USB_CABLE_CONNECTED) {
            if (psp_usb_cable_connection == false)
                Utils::StartUSBStorage();
        }
        else {
            if (psp_usb_cable_connection == true)
                Utils::StopUSBStorage();
        }
    }

    bool IsModelPSPGo(void) {
        return (kuKernelGetModel() == 4);
    }

    int IsMemCardInserted(bool &is_inserted) {
        int status = 0, ret = 0;
        if (R_FAILED(ret = sceIoDevctl("mscmhc0:", 0x02025806, 0, 0, &status, sizeof(status))))
            return ret;
            
        if (status != 1)
            is_inserted = false;
        else
            is_inserted = true;
        
        return 0;
    }

    bool IsInternalStorage(void) {
        if (is_psp_go) {
            if (!is_ms_inserted)
                return true;
                
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
        
        if (R_FAILED(ret = sctrlKernelLoadExecVSHWithApitype(Utils::IsInternalStorage()? 0x152 : 0x141, path, &param))) {
            Log::Error("sctrlKernelLoadExecVSHWithApitype(%x, %s) failed: 0x%08x\n", Utils::IsInternalStorage()? 0x152 : 0x141, path, ret);
            return ret;
        }
        
        return 0;
    }

    u64 GetTotalStorage(void) {
        int ret = 0;
        SystemDevCtl devctl;
        SystemDevCommand command;
        command.pdevinf = &devctl;
        
        if (R_FAILED(ret = sceIoDevctl(device == BROWSE_STATE_INTERNAL? "ef0": "ms0:", 0x02425818, &command, sizeof(SystemDevCommand), nullptr, 0)))
            return 0;
            
        u64 size = (devctl.maxclusters * devctl.sectorcount) * devctl.sectorsize;
        return size;
    }

    u64 GetFreeStorage(void) {
        int ret = 0;
        SystemDevCtl devctl;
        SystemDevCommand command;
        command.pdevinf = &devctl;
        
        if (R_FAILED(ret = sceIoDevctl(device == BROWSE_STATE_INTERNAL? "ef0": "ms0:", 0x02425818, &command, sizeof(SystemDevCommand), nullptr, 0)))
            return 0;
            
        u64 size = (devctl.freeclusters * devctl.sectorcount) * devctl.sectorsize; 
        return size;
    }

    u64 GetUsedStorage(void) {
        return (Utils::GetTotalStorage() - Utils::GetFreeStorage());
    }
    
    static int GetRegistryValue(const char *dir, const char *name, unsigned int *value) {
        int ret = 0;
        struct RegParam reg_param;
        REGHANDLE reg_handle = 0, reg_handle_cat = 0, reg_handle_key = 0;
        unsigned int type = 0, size = 0;
        
        std::memset(&reg_param, 0, sizeof(RegParam));
        reg_param.regtype = 1;
        reg_param.namelen = std::strlen("/system");
        reg_param.unk2 = 1;
        reg_param.unk3 = 1;
        std::strcpy(reg_param.name, "/system");

        if (R_FAILED(ret = sceRegOpenRegistry(&reg_param, 2, &reg_handle))) {
            Log::Error("sceRegOpenRegistry() failed: 0x%08x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = sceRegOpenCategory(reg_handle, dir, 2, &reg_handle_cat))) {
            sceRegCloseRegistry(reg_handle);
            Log::Error("sceRegOpenCategory() failed: 0x%08x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = sceRegGetKeyInfo(reg_handle_cat, name, &reg_handle_key, &type, &size))) {
            sceRegCloseCategory(reg_handle_cat);
            sceRegCloseRegistry(reg_handle);
            Log::Error("sceRegGetKeyInfo() failed: 0x%08x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = sceRegGetKeyValue(reg_handle_cat, reg_handle_key, value, 4))) {
            sceRegCloseCategory(reg_handle_cat);
            sceRegCloseRegistry(reg_handle);
            Log::Error("sceRegGetKeyValue() failed: 0x%08x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = sceRegFlushCategory(reg_handle_cat))) {
            sceRegCloseCategory(reg_handle_cat);
            sceRegCloseRegistry(reg_handle);
            Log::Error("sceRegFlushCategory() failed: 0x%08x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = sceRegCloseCategory(reg_handle_cat))) {
            sceRegCloseRegistry(reg_handle);
            Log::Error("sceRegCloseCategory() failed: 0x%08x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = sceRegFlushRegistry(reg_handle))) {
            sceRegCloseRegistry(reg_handle);
            Log::Error("sceRegFlushRegistry() failed: 0x%08x\n", ret);
            return ret;
        }

        if (R_FAILED(ret = sceRegCloseRegistry(reg_handle))) {
            Log::Error("sceRegFlushRegistry() failed: 0x%08x\n", ret);
            return ret;
        }
            
        return 0;
    }
    
    int ReadControls(void) {
        prev_pad = pad;
        sceCtrlReadBufferPositive(&pad, 1);
        
        if (pad.Buttons == last_button) {
            if (pad.TimeStamp - deadzone_tick < CTRL_DEADZONE_DELAY)
                return 0;
                
            if (pad.TimeStamp - last_button_tick < CTRL_DELAY)
                return 0;
                
            last_button_tick = pad.TimeStamp;
            return last_button;
        }
        
        last_button = pad.Buttons;
        deadzone_tick = last_button_tick = pad.TimeStamp;
        return last_button;
    }
    
    int IsButtonPressed(enum PspCtrlButtons buttons) {
        return ((pad.Buttons & buttons) == buttons) && ((prev_pad.Buttons & buttons) != buttons);
    }
    
    int IsButtonHeld(enum PspCtrlButtons buttons) {
        return pad.Buttons & buttons;
    }
    
    enum PspCtrlButtons GetEnterButton(void) {
        int ret = 0, button = -1;

        if (R_FAILED(ret = sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &button))) {
            Log::Error("sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN) failed: 0x%08x\n", ret);

            unsigned int reg_button = -1;
            if (R_SUCCEEDED(Utils::GetRegistryValue("/CONFIG/SYSTEM/XMB", "button_assign", &reg_button))) {
                if (reg_button == 0)
                    return PSP_CTRL_CIRCLE;
                
                return PSP_CTRL_CROSS;
            }
        }

        if (button == 0)
            return PSP_CTRL_CIRCLE;
            
        return PSP_CTRL_CROSS; // By default return PSP_CTRL_CROSS
    }
    
    // Basically the opposite of GetEnterButton()
    enum PspCtrlButtons GetCancelButton(void) {
        int ret = 0, button = -1;

        if (R_FAILED(ret = sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &button))) {
            Log::Error("sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN) failed: 0x%08x\n", ret);

            unsigned int reg_button = -1;
            if (R_SUCCEEDED(Utils::GetRegistryValue("/CONFIG/SYSTEM/XMB", "button_assign", &reg_button))) {
                if (reg_button == 0)
                    return PSP_CTRL_CROSS;
                
                return PSP_CTRL_CIRCLE;
            }
        }

        if (button == 0)
            return PSP_CTRL_CROSS;
            
        return PSP_CTRL_CIRCLE; // By default return PSP_CTRL_CIRCLE
    }
    
    float GetAnalogX(void) {
        return ((static_cast<float>(pad.Lx - 122.5f)) / 122.5f);
    }
    
    float GetAnalogY(void) {
        return ((static_cast<float>(pad.Ly - 122.5f)) / 122.5f);
    }
    
    bool IsCancelButtonPressed(void) {
        Utils::ReadControls();
        
        if (Utils::IsButtonPressed(PSP_CTRL_CANCEL))
            return true;
            
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
