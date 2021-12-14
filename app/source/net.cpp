#include <cstring>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspiofilemgr.h>
#include <pspnet.h>
#include <pspnet_apctl.h>
#include <pspnet_inet.h>
#include <psppower.h>
#include <pspumd.h>
#include <psputility.h>

#include "config.h"
#include "ftppsp.h"
#include "g2d.h"
#include "log.h"
#include "utils.h"

namespace Flash {
    void Init(void) {
        unsigned int ret = 0;
        
        if ((R_FAILED(ret = sceIoUnassign("flash0:"))) && (ret != 0x80020321))
            Log::Error("sceIoUnassign(flash0) failed: 0x%x\n", ret);
            
        if (R_FAILED(ret = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, nullptr, 0)))
            Log::Error("sceIoAssign(flash0) failed: 0x%x\n", ret);
            
        if ((R_FAILED(ret = sceIoUnassign("flash1:"))) && (ret != 0x80020321))
            Log::Error("sceIoUnassign(flash1) failed: 0x%x\n", ret);
            
        if (R_FAILED(ret = sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, nullptr, 0)))
            Log::Error("sceIoAssign(flash1) failed: 0x%x\n", ret);
            
        if ((R_FAILED(ret = sceIoUnassign("flash2:"))) && (ret != 0x80020321))
            Log::Error("sceIoUnassign(flash2) failed: 0x%x\n", ret);
            
        if (R_FAILED(ret = sceIoAssign("flash2:", "lflash0:0,2", "flashfat2:", IOASSIGN_RDWR, nullptr, 0)))
            Log::Error("sceIoAssign(flash2) failed: 0x%x\n", ret);
            
        if ((R_FAILED(ret = sceIoUnassign("flash3:"))) && (ret != 0x80020321))
            Log::Error("sceIoUnassign(flash3) failed: 0x%x\n", ret);
        
        if (R_FAILED(ret = sceIoAssign("flash3:", "lflash0:0,3", "flashfat3:", IOASSIGN_RDWR, nullptr, 0)))
            Log::Error("sceIoAssign(flash3) failed: 0x%x\n", ret);
    }
    
    void Exit(void) {
        unsigned int ret = 0;
        
        if ((R_FAILED(ret = sceIoUnassign("flash0:"))) && (ret != 0x80020321))
            Log::Error("sceIoUnassign(flash0) failed: 0x%x\n", ret);
        
        if (R_FAILED(ret = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDONLY, nullptr, 0)))
            Log::Error("sceIoAssign(flash0) failed: 0x%x\n", ret);
            
        if ((R_FAILED(ret = sceIoUnassign("flash1:"))) && (ret != 0x80020321))
            Log::Error("sceIoUnassign(flash1) failed: 0x%x\n", ret);
            
        if (R_FAILED(ret = sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDONLY, nullptr, 0)))
            Log::Error("sceIoAssign(flash1) failed: 0x%x\n", ret);
            
        if ((R_FAILED(ret = sceIoUnassign("flash2:"))) && (ret != 0x80020321))
            Log::Error("sceIoUnassign(flash2) failed: 0x%x\n", ret);
            
        if (R_FAILED(ret = sceIoAssign("flash2:", "lflash0:0,2", "flashfat2:", IOASSIGN_RDONLY, nullptr, 0)))
            Log::Error("sceIoAssign(flash2) failed: 0x%x\n", ret);
            
        if ((R_FAILED(ret = sceIoUnassign("flash3:"))) && (ret != 0x80020321))
            Log::Error("sceIoUnassign(flash3) failed: 0x%x\n", ret);
            
        if (R_FAILED(ret = sceIoAssign("flash3:", "lflash0:0,3", "flashfat3:", IOASSIGN_RDONLY, nullptr, 0)))
            Log::Error("sceIoAssign(flash3) failed: 0x%x\n", ret);
    }
}

namespace Net {
    static int DisplayNetDialog(void) {
        int ret = 0;
        bool done = false;
        
        pspUtilityNetconfData data;
        std::memset(&data, 0, sizeof(pspUtilityNetconfData));
        
        data.base.size = sizeof(pspUtilityNetconfData);
        data.base.language = g_psp_language;
        data.base.buttonSwap = (PSP_CTRL_ENTER == PSP_CTRL_CROSS)? PSP_UTILITY_ACCEPT_CROSS : PSP_UTILITY_ACCEPT_CIRCLE;
        data.base.graphicsThread = 17;
        data.base.accessThread = 19;
        data.base.fontThread = 18;
        data.base.soundThread = 16;
        data.action = PSP_NETCONF_ACTION_CONNECTAP;
        data.hotspot = 0;
        
        struct pspUtilityNetconfAdhoc adhocparam;
        std::memset(&adhocparam, 0, sizeof(adhocparam));
        
        data.adhocparam = &adhocparam;
        
        if (R_FAILED(ret = sceUtilityNetconfInitStart(&data))) {
            Log::Error("sceUtilityNetconfInitStart() failed: 0x%08x\n", ret);
            return ret;
        }
        
        while(!done) {
            g2dClear(G2D_RGBA(39, 50, 56, 255));
            sceGuFinish();
            sceGuSync(0, 0);
            
            switch(sceUtilityNetconfGetStatus()) {
                case PSP_UTILITY_DIALOG_NONE:
                    done = true;
                    break;

                case PSP_UTILITY_DIALOG_VISIBLE:
                    if (R_FAILED(ret = sceUtilityNetconfUpdate(1))) {
                        Log::Error("sceUtilityNetconfUpdate(1) failed: 0x%08x\n", ret);
                        done = true;
                    }
                    break;
                
                case PSP_UTILITY_DIALOG_QUIT:
                    if (R_FAILED(ret = sceUtilityNetconfShutdownStart())) {
                        Log::Error("sceUtilityNetconfShutdownStart() failed: 0x%08x\n", ret);
                        done = true;
                    }
                    break;
                
                case PSP_UTILITY_DIALOG_FINISHED:
                    done = true;
                    break;
                    
                default:
                    break;
            }

            g2dFlip(G2D_VSYNC);
        }

        return 0;
    }
    
    static int Init(void) {
        int ret = 0;
        
        if (R_FAILED(ret = sceNetInit(128 * 1024, 42, 4 * 1024, 42, 4 * 1024))) {
            Log::Error("sceNetInit() failed: 0x%08x\n", ret);
            return ret;
        }
            
        if (R_FAILED(ret = sceNetInetInit())) {
            Log::Error("sceNetInetInit() failed: 0x%08x\n", ret);
            return ret;
        }
            
        if (R_FAILED(ret = sceNetApctlInit(0x8000, 48))) {
            Log::Error("sceNetApctlInit() failed: 0x%08x\n", ret);
            return ret;
        }
            
        return 0;
    }
    
    static void Exit(void) {
        sceNetApctlTerm();
        sceNetInetTerm();
        sceNetTerm();
    }

    bool IsConnected(void) {
        int ret = 0, state = PSP_NET_APCTL_STATE_DISCONNECTED;

        if (R_FAILED(ret  = sceNetApctlGetState(&state))) {
            Log::Error("sceNetApctlGetState() failed: 0x%08x\n", ret);
            return ret;
        }

        return (state == PSP_NET_APCTL_STATE_GOT_IP);
    }

    bool InitFTP(char *string) {
        int ret = 0;
        char psp_ip[16] = {0};
        unsigned short int psp_port = 0;

        scePowerLock(0);
        sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
        sceUtilityLoadNetModule(PSP_NET_MODULE_INET);

        if (R_FAILED(ret = Net::Init())) {
            snprintf(string, 27, "Net initialization Failed.");
            return false;
        }
        
        Net::DisplayNetDialog();

        if (R_FAILED(ret = ftppsp_init(psp_ip, &psp_port))) {
            snprintf(string, 27, "FTP initialization Failed.");
            return false;
        }
        if (is_psp_go) {
            if (is_ms_inserted) {
                ftppsp_add_device("ms0:");
                ftppsp_add_device("ef0:");
            }
            else
                ftppsp_add_device("ef0:");
        }
        else
            ftppsp_add_device("ms0:");

        if (cfg.dev_options) {
            Flash::Init();
            ftppsp_add_device("flash0:");
            ftppsp_add_device("flash1:");
            ftppsp_add_device("flash2:");
            ftppsp_add_device("flash3:");
            
            if (sceUmdCheckMedium() != 0) {
                if (R_FAILED(ret = sceUmdActivate(1, "disc0:")))
                    Log::Error("sceUmdActivate(disc0) failed: 0x%x\n", ret);
                
                if (R_FAILED(ret = sceUmdWaitDriveStat(PSP_UMD_READY)))
                    Log::Error("sceUmdWaitDriveStat() failed: 0x%x\n", ret);

                ftppsp_add_device("disc0:");
            }
        }
            
        if (ret < 0) {
            snprintf(string, 19, "Connection Failed.");
        }
        else
            snprintf(string, 36, "FTP Connected %s:%i", psp_ip, psp_port);

        return true;
    }

    void ExitFTP(void) {
        if (is_psp_go) {
            if (is_ms_inserted) {
                ftppsp_del_device("ms0:");
                ftppsp_del_device("ef0:");
            }
            else
                ftppsp_del_device("ef0:");
        }
        else
            ftppsp_del_device("ms0:");

        if (cfg.dev_options) {
            Flash::Exit();
            ftppsp_del_device("flash0:");
            ftppsp_del_device("flash1:");
            ftppsp_del_device("flash2:");
            ftppsp_del_device("flash3:");

            if (sceUmdCheckMedium() != 0) {
                int ret = 0;

                if (R_FAILED(ret = sceUmdDeactivate(1, "disc0:")))
                    Log::Error("sceUmdDeactivate(disc0) failed: 0x%x\n", ret);

                ftppsp_del_device("disc0:");
            }
        }
        
        ftppsp_fini();
        
        if (cfg.dev_options)
            Flash::Exit();
        
        Net::Exit();
        
        sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
        sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
        scePowerUnlock(0);
    }
}
