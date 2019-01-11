#include <oslib/oslib.h>
#include <pspkernel.h>

#include "common.h"
#include "config.h"
#include "fs.h"
#include "menus/menu_main.h"
#include "textures.h"
#include "utils.h"

PSP_MODULE_INFO("CMFileManager", PSP_MODULE_USER, VERSION_MAJOR, VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_MAX();

static int cpu_clock = 0, bus_clock = 0;

extern unsigned char Roboto_pgf_start[];
extern unsigned int Roboto_pgf_size;

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

	Utils_InitUSB();
	pspSdkInetInit();
	Config_Load();
	Config_GetLastDirectory();
	Textures_Load();

	OSL_VIRTUALFILENAME roboto_font_mem[] = {{"ram:/Roboto.pgf", Roboto_pgf_start, Roboto_pgf_size, &VF_MEMORY}};
	oslAddVirtualFileList(roboto_font_mem, oslNumberof(roboto_font_mem));

	font = oslLoadFontFile("ram:/Roboto.pgf");
	oslSetFont(font);

	OSL_KEYMASK_ENTER = Utils_GetEnterButton();
	OSL_KEYMASK_CANCEL = Utils_GetCancelButton();
}

static void Term_Services(void) {
	Textures_Free();
	pspSdkInetTerm();
	Utils_ExitUSB();
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
