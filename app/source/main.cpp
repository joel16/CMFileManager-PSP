#include <pspctrl.h>
#include <pspkernel.h>
#include <pspumd.h>

#include "config.h"
#include "g2d.h"
#include "gui.h"
#include "log.h"
#include "textures.h"
#include "utils.h"

PSP_MODULE_INFO("CMFileManager", 0x800, VERSION_MAJOR, VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_THRESHOLD_SIZE_KB(1024);
PSP_HEAP_SIZE_KB(-2048);

bool running = true;

namespace Services {
    int Init(void) {
        int ret = 0;

        sceCtrlSetSamplingCycle(0);
        sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
        Utils::InitKernelDrivers();
        Textures::Load();

        Utils::IsMemCardInserted(isMSInserted);
        isPSPGo = Utils::IsModelPSPGo();
        
        if (R_FAILED(ret = Config::Load())) {
            Log::Error("Config::Load failed: 0x%08x\n", ret);
            return ret;
        }
        
        G2D::LoadFonts();
        
        PSP_CTRL_ENTER = Utils::GetEnterButton();
        PSP_CTRL_CANCEL = Utils::GetCancelButton();
        language = Utils::GetLanguage();
        return 0;
    }
    
    void Exit(void) {
        if (sceUmdCheckMedium() != 0) {
            int ret = 0;
            
            if (R_FAILED(ret = sceUmdDeactivate(1, "disc0:"))) {
                Log::Error("sceUmdDeactivate(disc0) failed: 0x%x\n", ret);
            }
        }
        
        G2D::UnloadFonts();
        Textures::Free();
        Utils::TermKernelDrivers();
        sceKernelExitGame();
    }
    
    static int ExitCallback(int arg1, int arg2, void *common) {
        running = false;
        return 0;
    }
    
    static int CallbackThread(SceSize args, void *argp) {
        int callback = 0;
        callback = sceKernelCreateCallback("ExitCallback", Services::ExitCallback, nullptr);
        sceKernelRegisterExitCallback(callback);
        sceKernelSleepThreadCB();
        return 0;
    }
    
    int SetupCallbacks(void) {
        int thread = 0;
        
        if (R_SUCCEEDED(thread = sceKernelCreateThread("CallbackThread", Services::CallbackThread, 0x11, 0xFA0, 0, nullptr))) {
            sceKernelStartThread(thread, 0, 0);
        }
        
        return thread;
    }
}

int main(int argc, char* argv[]) {
    Services::SetupCallbacks();
    Services::Init();
    GUI::RenderLoop();
    Services::Exit();
}
