#include <pspkernel.h>
#include <pspsdk.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>

#include "systemctrl.h"

PSP_MODULE_INFO("CMFileManager Launcher", PSP_MODULE_USER, VERSION_MAJOR, VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_MAX();

static char EBOOT_PATH[264];

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

static int LaunchCMFIleManager(void) {
	struct SceKernelLoadExecVSHParam param;
	int ret = 0;

	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.key = "game";
	param.args = strlen(EBOOT_PATH + 1);
	param.argp = EBOOT_PATH;

	ret = sctrlKernelLoadExecVSHWithApitype(0x141, EBOOT_PATH, &param);
	
	return ret;
}

int main(int argc, char **argv) {
	Callbacks_Setup();

	char cwd[256];
	getcwd(cwd, 256);
	snprintf(EBOOT_PATH, 264, "%s/APP.PBP", cwd);

	LaunchCMFIleManager();
	sceKernelExitGame();
}
