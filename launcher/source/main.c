#include <pspkernel.h>
#include <pspsdk.h>
#include <string.h>

#include "kubridge.h"
#include "systemctrl.h"

PSP_MODULE_INFO("CMFileManager Launcher", PSP_MODULE_USER, VERSION_MAJOR, VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_MAX();

#define EBOOT_PATH_EF0 "ef0:/PSP/GAME/CMFileManager/APP.PBP"
#define EBOOT_PATH_MS0 "ms0:/PSP/GAME/CMFileManager/APP.PBP"
#define PSP_GO 4

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
	SceIoStat stat;
	u32 psp_model;

	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.key = "game";
	psp_model = kuKernelGetModel();

	if (psp_model == PSP_GO && sceIoGetstat(EBOOT_PATH_EF0, &stat) == 0) {
		param.args = strlen(EBOOT_PATH_EF0) + 1;
		param.argp = EBOOT_PATH_EF0;
		ret = sctrlKernelLoadExecVSHWithApitype(0x141, EBOOT_PATH_EF0, &param);
		return ret;
	}

	param.args = strlen(EBOOT_PATH_MS0 + 1);
	param.argp = EBOOT_PATH_MS0;
	ret = sctrlKernelLoadExecVSHWithApitype(0x141, EBOOT_PATH_MS0, &param);
	return ret;
}

int main(int argc, char **argv) {
	Callbacks_Setup();
	LaunchCMFIleManager();
	sceKernelExitGame();
}
