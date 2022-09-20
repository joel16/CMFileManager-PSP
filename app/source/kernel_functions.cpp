#include <pspaudio_kernel.h>

#include "kernel_functions.h"
#include "kubridge.h"
#include "log.h"
#include "systemctrl.h"
#include "utils.h"

// audio_driver functions
int pspAudioSetFrequency(int frequency) {
    struct KernelCallArg args = { 0 };
    void *func_addr = nullptr;
    int ret = 0;
    
    func_addr = (void *)sctrlHENFindFunction("sceAudio_driver", "sceAudio_driver", 0xA2BEAA6C);
    args.arg1 = static_cast<u32>(frequency);

    if (R_FAILED(ret = kuKernelCall(func_addr, &args)))
        Log::Error("%s: pspAudioSetFrequency returns 0x%08X\n", __func__, args.ret1);
        
    return args.ret1;
}

// display driver functions
int pspDisplayEnable(void);
int pspDisplayDisable(void);

// fs_driver functions
int pspIoOpenDir(const char *dirname);
int pspIoReadDir(SceUID dir, SceIoDirent *dirent);
int pspIoCloseDir(SceUID dir);
int pspIoMakeDir(const char *dir, SceMode mode);
int pspIoRemoveDir(const char *path);
int pspIoOpenFile(const char *file, int flags, SceMode mode);
int pspIoReadFile(SceUID file, void *data, SceSize size);
int pspIoWriteFile(SceUID file, void *data, SceSize size);
int pspIoCloseFile(SceUID file);
int pspIoLseek(SceUID file, SceOff offset, int whence);
int pspIoLseek32(SceUID file, SceOff offset, int whence);
int pspIoGetstat(const char *file, SceIoStat *stat);
int pspIoRename(const char *oldname, const char *newname);
int pspIoRemoveFile(const char *file);
int pspIoDevctl(const char *dev, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);

// module_driver functions
int pspKernelLoadModuleBuffer(SceSize size, void *buf, s32 flag, const SceKernelLMOption *option);
