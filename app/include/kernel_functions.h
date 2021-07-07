#ifndef _CMFILEMANAGER_KERNEL_FUNCTIONS_H_
#define _CMFILEMANAGER_KERNEL_FUNCTIONS_H_

#if defined (__cplusplus)
extern "C" {
#endif

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

// input_driver functions
extern unsigned int pspGetButtons(void);

#if defined (__cplusplus)
}
#endif

#endif
