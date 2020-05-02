#ifndef CMFILEMANAGER_KERNEL_FUNCTIONS_H
#define CMFILEMANAGER_KERNEL_FUNCTIONS_H

#include <pspctrl.h>
#include <pspiofilemgr.h>

// Kernel function prototypes

// audio_driver functions
extern int pspAudioSetFrequency(int frequency);

// display_driver functions
extern int pspGetBrightness(int *brightness);
extern int pspSetBrightness(int brightness);
extern int pspDisplayEnable(void);
extern int pspDisplayDisable(void);

// fs_driver functions
extern int pspOpenDir(const char *dirname);
extern int pspReadDir(SceUID dir, SceIoDirent *dirent);
extern int pspCloseDir(SceUID dir);

// impose_driver functions
extern int pspGetVolume(void);
extern int pspSetVolume(int volume);
extern int pspGetMute(void);
extern int pspSetMute(int mute);
extern int pspGetEqualizerMode(void);
extern int pspSetEqualizerMode(int mode);
extern int pspSetHomePopup(int popup);

// input_driver functions
extern unsigned int pspGetButtons(void);

#endif
