#include <oslib/oslib.h>
#include <pspkernel.h>

#include "common.h"
#include "config.h"
#include "fs.h"
#include "menus/menu_main.h"
#include "textures.h"

PSP_MODULE_INFO("CMFileManager", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-1);

static int cpu_clock = 0, bus_clock = 0;

static void Init_Oslib(void) {
	oslInit(0);
	oslInitGfx(OSL_PF_8888, 1);
	oslInitAudio();
	oslSetQuitOnLoadFailure(1);
	oslSetKeyAutorepeatInit(40);
	oslSetKeyAutorepeatInterval(10);
	oslIntraFontInit(INTRAFONT_CACHE_MED);
}

static void Term_Oslib(void) {
	oslIntraFontShutdown();
	oslDeinitAudio();
	oslEndGfx();
}

int main(int argc, char **argv) {
	// Set to max clock frequency.
	cpu_clock = scePowerGetCpuClockFrequency();
	bus_clock = scePowerGetBusClockFrequency();
	scePowerSetClockFrequency(333, 333, 167);

	Init_Oslib();
	pspSdkInetInit();
	Config_Load();
	Textures_Load();

	font = oslLoadFontFile("data/Roboto.pgf");
	oslSetFont(font);

	if (FS_FileExists("lastdir.txt")) {
		char *buf = (char *)malloc(256);
		FS_ReadFile("lastdir.txt", buf, 256);
		sscanf(buf, "%s", buf);
		
		if (FS_DirExists(buf)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
			strcpy(cwd, buf);
		else 
			strcpy(cwd, START_PATH);

		free(buf);
	}
	else {
		char *buf = (char *)malloc(256);
		int len = snprintf(buf, 256, START_PATH);
		FS_WriteFile("lastdir.txt", buf, len);
		strcpy(cwd, buf); // Set Start Path to "ms0:/" if lastDir.txt hasn't been created.
		free(buf);
	}

	Menu_Main();

	Textures_Free();
	pspSdkInetTerm();
	Term_Oslib();
	scePowerSetClockFrequency(cpu_clock, cpu_clock, bus_clock);
	oslQuit();
	return 0;
}
