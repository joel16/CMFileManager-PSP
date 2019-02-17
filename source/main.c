#include <pspkernel.h>
#include <psppower.h>
#include <pspsdk.h>

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

extern const char Roboto_pgf_start[];
extern unsigned int Roboto_pgf_size;

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

static void Init_Services(void) {
	Callbacks_Setup();

	// Set to max clock frequency.
	cpu_clock = scePowerGetCpuClockFrequency();
	bus_clock = scePowerGetBusClockFrequency();
	scePowerSetClockFrequency(333, 333, 167);

	intraFontInit();

	Utils_InitUSB();
	pspSdkInetInit();
	Config_Load();
	Config_GetLastDirectory();
	Textures_Load();

	font = intraFontLoadMem("ram:/Roboto.pgf", Roboto_pgf_start, Roboto_pgf_size, INTRAFONT_CACHE_ALL);

	PSP_CTRL_ENTER = Utils_GetEnterButton();
	PSP_CTRL_CANCEL = Utils_GetCancelButton();
}

static void Term_Services(void) {
	Textures_Free();
	pspSdkInetTerm();
	Utils_ExitUSB();
	scePowerSetClockFrequency(cpu_clock, cpu_clock, bus_clock); // Restore previous clock frequency.
	intraFontShutdown();
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
