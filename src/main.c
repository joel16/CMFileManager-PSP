#include <oslib/oslib.h>
#include <pspkernel.h>

#include "common.h"
#include "config.h"
#include "fs.h"
#include "menus/menu_main.h"
#include "textures.h"

PSP_MODULE_INFO("CMFileManager", PSP_MODULE_USER, VERSION_MAJOR, VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_MAX();

static int cpu_clock = 0, bus_clock = 0;

static void Init_Services(void) {
	// Set to max clock frequency.
	cpu_clock = scePowerGetCpuClockFrequency();
	bus_clock = scePowerGetBusClockFrequency();
	scePowerSetClockFrequency(333, 333, 167);

	oslInit(0);
	oslInitGfx(OSL_PF_8888, 1);
	oslInitAudio();
	oslInitAudioME(OSL_FMT_ALL);
	oslSetQuitOnLoadFailure(1);
	oslSetKeyAutorepeatInit(40);
	oslSetKeyAutorepeatInterval(10);
	oslIntraFontInit(INTRAFONT_CACHE_LARGE | INTRAFONT_STRING_UTF8);

	oslInitUsbStorage();
	pspSdkInetInit();
	Config_Load();
	Config_GetLastDirectory();
	Textures_Load();

	font = oslLoadFontFile("data/Roboto.pgf");
	oslSetFont(font);
}

static void Term_Services(void) {
	Textures_Free();
	pspSdkInetTerm();
	oslDeinitUsbStorage();
	scePowerSetClockFrequency(cpu_clock, cpu_clock, bus_clock); // Restore previous clock frequency. 
	oslIntraFontShutdown();
	oslDeinitAudio();
	oslEndGfx();
	oslQuit();
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
