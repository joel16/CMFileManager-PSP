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
extern int pspDisplayEnable(void);
extern int pspDisplayDisable(void);

// fs_driver functions
extern int pspIoOpenDir(const char *dirname);
extern int pspIoReadDir(SceUID dir, SceIoDirent *dirent);
extern int pspIoCloseDir(SceUID dir);
extern int pspIoMakeDir(const char *dir, SceMode mode);
extern int pspIoRemoveDir(const char *path);
extern int pspIoOpenFile(const char *file, int flags, SceMode mode);
extern int pspIoReadFile(SceUID file, void *data, SceSize size);
extern int pspIoWriteFile(SceUID file, void *data, SceSize size);
extern int pspIoCloseFile(SceUID file);
extern int pspIoLseek(SceUID file, SceOff offset, int whence);
extern int pspIoLseek32(SceUID file, SceOff offset, int whence);
extern int pspIoGetstat(const char *file, SceIoStat *stat);
extern int pspIoRename(const char *oldname, const char *newname);
extern int pspIoRemoveFile(const char *file);
extern int pspIoDevctl(const char *dev, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);

// input_driver functions
extern unsigned int pspGetButtons(void);

#if defined (__cplusplus)
}
#endif

#endif
