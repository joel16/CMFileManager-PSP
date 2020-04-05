#include <pspsdk.h>
#include <pspctrl.h>

PSP_MODULE_INFO("input_driver", PSP_MODULE_KERNEL, 1, 0);
PSP_NO_CREATE_MAIN_THREAD();

SceCtrlData pspGetButtons(void) {
	u32 k1 = pspSdkSetK1(0);
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(&pad, 1);
	pspSdkSetK1(k1);
	return pad;
}

int module_start(SceSize args, void *argp) {
	return 0;
}

int module_stop(void) {
	return 0;
}
