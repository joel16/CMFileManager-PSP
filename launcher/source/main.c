#include <pspsdk.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>

#include "systemctrl.h"

PSP_MODULE_INFO("CMFileManager Launcher", PSP_MODULE_USER, VERSION_MAJOR, VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

int main(int argc, char **argv) {
	int ret = 0;
	char path[264], cwd[256];
	struct SceKernelLoadExecVSHParam param;

	getcwd(cwd, 256);
	snprintf(path, 264, "%s/APP.PBP", cwd);
	
	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.key = "game";
	param.args = strlen(path + 1);
	param.argp = path;

	ret = sctrlKernelLoadExecVSHWithApitype(0x141, path, &param);

	sceKernelExitGame();
	return ret;
}
