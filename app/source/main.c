#include <pspctrl.h>
#include <pspkernel.h>
#include <psppower.h>
#include <sys/unistd.h>

#include "common.h"
#include "config.h"
#include "log.h"
#include "menus/menu_main.h"
#include "kubridge.h"
#include "systemctrl.h"
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

	Utils_IsMemCardInserted(&is_ms_inserted);
	is_psp_go = Utils_IsModelPSPGo();

	Log_OpenFileHande();

	Utils_InitKernelDrivers();
	
	if (R_FAILED(ret = Config_Load())) {
		Log_Print("Config_Load failed: 0x%lx\n", ret);
		return ret;
	}

	if (R_FAILED(ret = Config_GetLastDirectory())) {
		Log_Print("Config_GetLastDirectory failed: 0x%lx\n", ret);
		return ret;
	}

	Textures_Load();

	if (R_FAILED(ret = intraFontInit())) {
		Log_Print("intraFontInit failed: 0x%lx\n", ret);
		return ret;
	}

	font = intraFontLoadMem("ram:/Roboto.pgf", Roboto_pgf_start, Roboto_pgf_size, INTRAFONT_CACHE_ALL);

	PSP_CTRL_ENTER = Utils_GetEnterButton();
	PSP_CTRL_CANCEL = Utils_GetCancelButton();
	return 0;
}

static void Term_Services(void) {
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
