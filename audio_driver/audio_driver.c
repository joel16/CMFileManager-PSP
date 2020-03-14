#include <pspsdk.h>
#include <pspaudio_kernel.h>

PSP_MODULE_INFO("audio_driver", PSP_MODULE_KERNEL, 1, 2);
PSP_NO_CREATE_MAIN_THREAD();

int pspAudioSetFrequency(int frequency) {
	u32 k1 = pspSdkSetK1(0);
	int ret = sceAudioSetFrequency(frequency);
	pspSdkSetK1(k1);
	return ret;
}

int module_start(SceSize args, void *argp) {
	return 0;
}

int module_stop(void) {
	return 0;
}
