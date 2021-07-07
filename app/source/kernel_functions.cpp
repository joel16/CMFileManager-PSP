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

// input_driver functions
unsigned int pspGetButtons(void);
