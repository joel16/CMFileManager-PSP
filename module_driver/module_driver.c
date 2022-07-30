#include <pspsdk.h>

PSP_MODULE_INFO("module_driver", PSP_MODULE_KERNEL, 1, 0);
PSP_NO_CREATE_MAIN_THREAD();

SceUID sceKernelLoadModuleBufferForKernel(SceSize size, void *buf, s32 flag, const SceKernelLMOption *option);

int pspKernelLoadModuleBuffer(SceSize size, void *buf, s32 flag, const SceKernelLMOption *option) {
	int k1 = pspSdkSetK1(0);
	int ret = sceKernelLoadModuleBufferForKernel(size, buf, flag, option);
	pspSdkSetK1(k1);
	return ret;
}

int module_start(SceSize args, void *argp) {
	return 0;
}

int module_stop(void) {
	return 0;
}
