#include <pspsdk.h>
#include <pspaudio_kernel.h>

PSP_MODULE_INFO("audio_driver", PSP_MODULE_KERNEL, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

int pspAudioSetFrequency(int frequency) {
	u32 k1 = 0;
	k1 = pspSdkSetK1(0);

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
