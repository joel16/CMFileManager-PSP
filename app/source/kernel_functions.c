#include "kernel_functions.h"

// audio_driver functions
int pspAudioSetFrequency(int frequency);

// display driver functions
int pspGetBrightness(int *brightness);
int pspSetBrightness(int brightness);
int pspDisplayEnable(void);
int pspDisplayDisable(void);

// fs_driver functions
int pspOpenDir(const char *dirname);
int pspReadDir(SceUID dir, SceIoDirent *dirent);
int pspCloseDir(SceUID dir);

// impose_driver functions
int pspGetVolume(void);
int pspSetVolume(int volume);
int pspGetMute(void);
int pspSetMute(int mute);
int pspGetEqualizerMode(void);
int pspSetEqualizerMode(int mode);
int pspSetHomePopup(int popup);

// input_driver functions
SceCtrlData pspGetButtons(void);
