#include <pspsdk.h>
#include <pspiofilemgr_kernel.h>

#include "systemctrl.h"

PSP_MODULE_INFO("fs_driver", PSP_MODULE_KERNEL, 1, 1);
PSP_NO_CREATE_MAIN_THREAD();

int pspOpenDir(const char *dirname) {
	u32 k1 = 0;
	int oldlevel = sctrlKernelSetUserLevel(4);
	k1 = pspSdkSetK1(0);
	
	int ret = sceIoDopen(dirname);
	
	pspSdkSetK1(k1);
	sctrlKernelSetUserLevel(oldlevel);
	return ret;
}

int pspReadDir(SceUID dir, SceIoDirent *dirent) {
	u32 k1 = pspSdkSetK1(0);
	int ret = sceIoDread(dir, dirent);
	pspSdkSetK1(k1);
	return ret;
}

int pspCloseDir(SceUID dir) {
	u32 k1 = pspSdkSetK1(0);
	int ret = sceIoDclose(dir);
	pspSdkSetK1(k1);
	return ret;
}

int module_start(SceSize args, void *argp) {
	return 0;
}

int module_stop(void) {
	return 0;
}
