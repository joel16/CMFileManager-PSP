#include <pspsdk.h>
#include <pspimpose_driver.h>

PSP_MODULE_INFO("impose_driver", PSP_MODULE_KERNEL, 1, 1);
PSP_NO_CREATE_MAIN_THREAD();

int pspGetVolume(void) {
	u32 k1 = pspSdkSetK1(0);
	int ret = sceImposeGetParam(PSP_IMPOSE_MAIN_VOLUME);
	pspSdkSetK1(k1);
	return ret;
}

int pspSetVolume(int volume) {
	u32 k1 = pspSdkSetK1(0);
	int ret = sceImposeSetParam(PSP_IMPOSE_MAIN_VOLUME, volume);
	pspSdkSetK1(k1);
	return ret;
}

int pspGetMute(void) {
	u32 k1 = pspSdkSetK1(0);
	int ret = sceImposeGetParam(PSP_IMPOSE_MUTE);
	pspSdkSetK1(k1);
	return ret;
}

int pspSetMute(int mute) {
	u32 k1 = pspSdkSetK1(0);
	int ret = sceImposeSetParam(PSP_IMPOSE_MUTE, mute);
	pspSdkSetK1(k1);
	return ret;
}

int pspGetEqualizerMode(void) {
	u32 k1 = pspSdkSetK1(0);
	int ret = sceImposeGetParam(PSP_IMPOSE_EQUALIZER_MODE);
	pspSdkSetK1(k1);
	return ret;
}

int pspSetEqualizerMode(int mode) {
	u32 k1 = pspSdkSetK1(0);
	int ret = sceImposeSetParam(PSP_IMPOSE_EQUALIZER_MODE, mode);
	pspSdkSetK1(k1);
	return ret;
}

int pspSetHomePopup(int popup) {
	u32 k1 = pspSdkSetK1(0);
	int ret = sceImposeSetHomePopup(popup);
	pspSdkSetK1(k1);
	return ret;
}

int module_start(SceSize args, void *argp) {
	return 0;
}

int module_stop(void) {
	return 0;
}
