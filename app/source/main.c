#include <pspctrl.h>
#include <pspkernel.h>
#include <psppower.h>
#include <sys/unistd.h>

#include "common.h"
#include "config.h"
#include "kernel_functions.h"
#include "kubridge.h"
#include "log.h"
#include "menus/menu_main.h"
#include "systemctrl.h"
#include "textures.h"
#include "utils.h"

PSP_MODULE_INFO("CMFileManager", 0x800, VERSION_MAJOR, VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-2048);

static int cpu_clock = 0, bus_clock = 0;

extern const char NotoSans_pgf_start[];
extern unsigned int NotoSans_pgf_size;

static int Callbacks_Exit() {
	sceKernelExitGame();
	return 0;
}
  
static int Callbacks_Thread() {
	int id = 0;
	id = sceKernelCreateCallback("exit_cb", Callbacks_Exit, NULL);
	sceKernelRegisterExitCallback(id);
	sceKernelSleepThreadCB();
	return 0;
}
  
static int Callbacks_Setup(void) {
	int id = 0;
	id = sceKernelCreateThread("cb", Callbacks_Thread, 0x11, 0xFA0, 0, NULL);

	if (id >= 0)
		sceKernelStartThread(id, 0, NULL);

	return id;
}

static int Init_Services(void) {
	int ret = 0;
	Callbacks_Setup();

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

	// Set to max clock frequency.
	cpu_clock = scePowerGetCpuClockFrequency();
	bus_clock = scePowerGetBusClockFrequency();
	if (R_FAILED(ret = scePowerSetClockFrequency(333, 333, 166)))
		return ret;

	// Get the initial working directory.
	getcwd(initial_cwd, 128);
	
	Utils_InitKernelDrivers();
	Log_OpenFileHande();
	Textures_Load();

	if (R_FAILED(ret = intraFontInit())) {
		Log_Print("intraFontInit failed: 0x%lx\n", ret);
		return ret;
	}

	font = intraFontLoadMem("ram:/NotoSans.pgf", NotoSans_pgf_start, NotoSans_pgf_size, INTRAFONT_STRING_UTF8 | INTRAFONT_CACHE_LARGE);
	jpn_font = intraFontLoadMem("ram:/NotoSans.pgf", NotoSans_pgf_start, NotoSans_pgf_size, INTRAFONT_STRING_UTF8 | INTRAFONT_CACHE_LARGE);
	chn_font = intraFontLoadMem("ram:/NotoSans.pgf", NotoSans_pgf_start, NotoSans_pgf_size, INTRAFONT_STRING_UTF8 | INTRAFONT_CACHE_LARGE);
	kor_font = intraFontLoadMem("ram:/NotoSans.pgf", NotoSans_pgf_start, NotoSans_pgf_size, INTRAFONT_STRING_UTF8 | INTRAFONT_CACHE_LARGE);
	sym_font = intraFontLoadMem("ram:/NotoSans.pgf", NotoSans_pgf_start, NotoSans_pgf_size, INTRAFONT_STRING_UTF8 | INTRAFONT_CACHE_LARGE);

	intraFontSetAltFont(font, jpn_font);
	intraFontSetAltFont(jpn_font, chn_font);
	intraFontSetAltFont(chn_font, kor_font);
	intraFontSetAltFont(kor_font, sym_font);

	Utils_IsMemCardInserted(&is_ms_inserted);
	is_psp_go = Utils_IsModelPSPGo();
	
	if (R_FAILED(ret = Config_Load())) {
		Log_Print("Config_Load failed: 0x%lx\n", ret);
		return ret;
	}

	PSP_CTRL_ENTER = Utils_GetEnterButton();
	PSP_CTRL_CANCEL = Utils_GetCancelButton();
	pspSetHomePopup(0);
	return 0;
}

static void Term_Services(void) {
	pspSetHomePopup(1);
	intraFontUnload(sym_font);
	intraFontUnload(kor_font);
	intraFontUnload(chn_font);
	intraFontUnload(jpn_font);
	intraFontUnload(font);
	Textures_Free();
	Utils_TermKernelDrivers();
	Log_CloseFileHandle();
	scePowerSetClockFrequency(cpu_clock, cpu_clock, bus_clock); // Restore previous clock frequency.
	sceKernelExitGame();
}

int main(int argc, char **argv) {
	Init_Services();

	if (setjmp(exitJmp)) {
		Term_Services();
		return 0;
	}

	Menu_Main();
	Term_Services();

	return 0;
}
