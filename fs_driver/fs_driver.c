#include <pspsdk.h>
#include <pspiofilemgr_kernel.h>

PSP_MODULE_INFO("audio_driver", PSP_MODULE_KERNEL, 1, 1);
PSP_NO_CREATE_MAIN_THREAD();

int fsOpenDir(const char *dirname) {
	u32 k1 = 0;
	k1 = pspSdkSetK1(0);

	int ret = sceIoDopen(dirname);

	pspSdkSetK1(k1);
	return ret;
}

int fsReadDir(SceUID dir, SceIoDirent *dirent) {
	u32 k1 = 0;
	k1 = pspSdkSetK1(0);

	int ret = sceIoDread(dir, dirent);

	pspSdkSetK1(k1);
	return ret;
}

int fsCloseDir(SceUID dir) {
	u32 k1 = 0;
	k1 = pspSdkSetK1(0);

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
