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
PSP_HEAP_SIZE_KB(-2048);

bool g_running = true;

namespace Services {
    int Init(void) {
        sceCtrlSetSamplingCycle(0);
        sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
        Utils::InitKernelDrivers();
        Textures::Load();

        int ret = 0;
        if (R_FAILED(ret = Config::Load())) {
			Log::Error("Config::Load failed: 0x%08x\n", ret);
			return ret;
		}

		if (R_FAILED(ret = intraFontInit())) {
			Log::Error("intraFontInit failed: 0x%08x\n", ret);
			return ret;
		}

        font = intraFontLoad("flash0:/font/ltn8.pgf", INTRAFONT_CACHE_ALL);
        jpn0 = intraFontLoad("flash0:/font/jpn0.pgf", INTRAFONT_STRING_UTF8);
        chn = intraFontLoad("flash0:/font/gb3s1518.bwfon", 0);
        intraFontSetAltFont(font, jpn0);
        intraFontSetAltFont(jpn0, chn);
        intraFontSetEncoding(font, INTRAFONT_STRING_UTF8);

        // Font size cache
        for (int i = 0; i < 256; i++) {
            char character[2] = {0};
            character[0] = i;
            character[1] = '\0';
            font_size_cache[i] = intraFontMeasureText(font, character);
        }

        Utils::IsMemCardInserted(is_ms_inserted);
		is_psp_go = Utils::IsModelPSPGo();
		
		PSP_CTRL_ENTER = Utils::GetEnterButton();
		PSP_CTRL_CANCEL = Utils::GetCancelButton();
        g_psp_language = Utils::GetLanguage();
        return 0;
    }

    void Exit(void) {
        if (sceUmdCheckMedium() != 0) {
            int ret = 0;
            
            if (R_FAILED(ret = sceUmdDeactivate(1, "disc0:")))
                Log::Error("sceUmdDeactivate(disc0) failed: 0x%x\n", ret);
        }

        intraFontUnload(chn);
        intraFontUnload(jpn0);
        intraFontUnload(font);
        Textures::Free();
        Utils::TermKernelDrivers();
        sceKernelExitGame();
    }
    
    static int ExitCallback(int arg1, int arg2, void *common) {
        g_running = false;
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

        if (R_SUCCEEDED(thread = sceKernelCreateThread("CallbackThread", Services::CallbackThread, 0x11, 0xFA0, 0, nullptr)))
            sceKernelStartThread(thread, 0, 0);
            
        return thread;
    }
}

int main(int argc, char* argv[]) {
    Services::SetupCallbacks();
    Services::Init();
    GUI::RenderLoop();
    Services::Exit();
}
