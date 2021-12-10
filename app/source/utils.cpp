#include <cstdio>
#include <cstring>
#include <pspkernel.h>
#include <pspreg.h>
#include <pspusb.h>
#include <pspusbstor.h>
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

namespace Utils {
    constexpr unsigned int CTRL_DEADZONE_DELAY = 500000;
    constexpr unsigned int CTRL_DELAY = 100000;

    static SceCtrlData pad, kernel_pad, prev_pad;
    static unsigned int last_button = 0;
    static int last_button_tick = 0, deadzone_tick = 0;
    static bool usb_module_loaded = false;
    static bool usb_actived = false;
    
    typedef struct {
        const char *path = nullptr;
        int id = 0;
    } Module;
    
    static std::vector<Module> kernel_modules {
        { "audio_driver.prx", -1, },
        { "display_driver.prx", -1, },
        { "fs_driver.prx", -1, },
        { "input_driver.prx", -1, }
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
        
        while (size >= 1024.0f) {
            size /= 1024.0f;
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
            kernel_modules[i].id = Utils::LoadStartModule(kernel_modules[i].path);
        
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
        struct RegParam reg;
        REGHANDLE h;
        
        std::memset(&reg, 0, sizeof(reg));
        reg.regtype = 1;
        reg.namelen = std::strlen("/system");
        reg.unk2 = 1;
        reg.unk3 = 1;
        std::strcpy(reg.name, "/system");
        
        if (R_SUCCEEDED(sceRegOpenRegistry(&reg, 2, &h))) {
            REGHANDLE hd;
            
            if (R_SUCCEEDED(sceRegOpenCategory(h, dir, 2, &hd))) {
                REGHANDLE hk;
                unsigned int type, size;
                
                if (R_SUCCEEDED(sceRegGetKeyInfo(hd, name, &hk, &type, &size))) {
                    if (!sceRegGetKeyValue(hd, hk, value, 4)) {
                        ret = 1;
                        sceRegFlushCategory(hd);
                    }
                }

                sceRegCloseCategory(hd);
            }

            sceRegFlushRegistry(h);
            sceRegCloseRegistry(h);
        }
        
        return ret;
    }
    
    int ReadControls(void) {
        prev_pad = pad;
        kernel_pad.Buttons = pspGetButtons();
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
    
    int IsKButtonPressed(enum PspCtrlButtons buttons) {
        return ((kernel_pad.Buttons & buttons) == buttons) && ((prev_pad.Buttons & buttons) != buttons);
    }
    
    int IsKButtonHeld(enum PspCtrlButtons buttons) {
        return kernel_pad.Buttons & buttons;
    }
    
    enum PspCtrlButtons GetEnterButton(void) {
        unsigned int button = 0;
        
        if (R_SUCCEEDED(GetRegistryValue("/CONFIG/SYSTEM/XMB", "button_assign", &button))) {
            if (button == 0)
                return PSP_CTRL_CIRCLE; // PSP_CTRL_CIRCLE
            else
                return PSP_CTRL_CROSS; // PSP_CTRL_CROSS
        }
        
        return PSP_CTRL_CROSS; // By default return PSP_CTRL_CROSS
    }
    
    // Basically the opposite of GetEnterButton()
    enum PspCtrlButtons GetCancelButton(void) {
        unsigned int button = 0;
        if (R_SUCCEEDED(GetRegistryValue("/CONFIG/SYSTEM/XMB", "button_assign", &button))) {
            if (button == 0)
                return PSP_CTRL_CROSS; // PSP_CTRL_CROSS
            else
                return PSP_CTRL_CIRCLE; // PSP_CTRL_CIRCLE
        }
        
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
}
