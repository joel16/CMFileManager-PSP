#include <pspctrl.h>
#include <pspkernel.h>

#include "config.h"
#include "g2d.h"
#include "gui.h"
#include "log.h"
#include "textures.h"
#include "utils.h"

PSP_MODULE_INFO("CMFileManager", 0x800, VERSION_MAJOR, VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-2048);

namespace Services {
    int Init(void) {
        sceCtrlSetSamplingCycle(0);
        sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
        Log::OpenHande();
        Utils::InitKernelDrivers();
        Textures::Load();

        int ret = 0;
        if (R_FAILED(ret = Config::Load())) {
			Log::Error("Config::Load failed: 0x%lx\n", ret);
			return ret;
		}

		if (R_FAILED(ret = intraFontInit())) {
			Log::Error("intraFontInit failed: 0x%lx\n", ret);
			return ret;
		}

        font = intraFontLoad("flash0:/font/ltn8.pgf", 0);
        jpn0 = intraFontLoad("flash0:/font/jpn0.pgf", INTRAFONT_STRING_UTF8);
        chn = intraFontLoad("flash0:/font/gb3s1518.bwfon", 0);
        intraFontSetAltFont(font, jpn0);
        intraFontSetAltFont(jpn0, chn);
        intraFontSetEncoding(font, INTRAFONT_STRING_UTF8);

        Utils::IsMemCardInserted(&is_ms_inserted);
		is_psp_go = Utils::IsModelPSPGo();
		
		PSP_CTRL_ENTER = static_cast<enum PspCtrlButtons>(Utils::GetEnterButton());
		PSP_CTRL_CANCEL = static_cast<enum PspCtrlButtons>(Utils::GetCancelButton());
        return 0;
    }

    void Exit(void) {
        intraFontUnload(chn);
        intraFontUnload(jpn0);
        intraFontUnload(font);
        Textures::Free();
        Log::CloseHandle();
        Utils::TermKernelDrivers();
        sceKernelExitGame();
    }
}

int main(int argc, char* argv[]) {
    Services::Init();
    GUI::RenderLoop();
    Services::Exit();
}
