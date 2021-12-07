#include <pspsdk.h>
#include <pspiofilemgr_kernel.h>

#include "systemctrl.h"

PSP_MODULE_INFO("fs_driver", PSP_MODULE_KERNEL, 1, 2);
PSP_NO_CREATE_MAIN_THREAD();

int pspIoOpenDir(const char *dirname) {
	u32 k1 = pspSdkSetK1(0);
	int level = sctrlKernelSetUserLevel(8);
	
	int ret = sceIoDopen(dirname);
	
	pspSdkSetK1(k1);
	sctrlKernelSetUserLevel(level);
	return ret;
}

int pspIoReadDir(SceUID dir, SceIoDirent *dirent) {
	u32 k1 = pspSdkSetK1(0);
	int level = sctrlKernelSetUserLevel(8);
	
	int ret = sceIoDread(dir, dirent);
	
	pspSdkSetK1(k1);
	sctrlKernelSetUserLevel(level);
	return ret;
}

int pspIoCloseDir(SceUID dir) {
	u32 k1 = pspSdkSetK1(0);
	int level = sctrlKernelSetUserLevel(8);
	
	int ret = sceIoDclose(dir);
	
	pspSdkSetK1(k1);
	sctrlKernelSetUserLevel(level);
	return ret;
}

int pspIoMakeDir(const char *dir, SceMode mode) {
	u32 k1 = pspSdkSetK1(0);
	int level = sctrlKernelSetUserLevel(8);
	
	int ret = sceIoMkdir(dir, mode);
	
	pspSdkSetK1(k1);
	sctrlKernelSetUserLevel(level);
	return ret;
}

int pspIoRemoveDir(const char *path) {
	u32 k1 = pspSdkSetK1(0);
	int level = sctrlKernelSetUserLevel(8);
	
	int ret = sceIoRmdir(path);
	
	pspSdkSetK1(k1);
	sctrlKernelSetUserLevel(level);
	return ret;
}

int pspIoOpenFile(const char *file, int flags, SceMode mode) {
	u32 k1 = pspSdkSetK1(0);
	int level = sctrlKernelSetUserLevel(8);
	
	int ret = sceIoOpen(file, flags, mode);
	
	pspSdkSetK1(k1);
	sctrlKernelSetUserLevel(level);
	return ret;
}

int pspIoReadFile(SceUID file, void *data, SceSize size) {
	u32 k1 = pspSdkSetK1(0);
	int level = sctrlKernelSetUserLevel(8);
	
	int ret = sceIoRead(file, data, size);
	
	pspSdkSetK1(k1);
	sctrlKernelSetUserLevel(level);
	return ret;
}

int pspIoWriteFile(SceUID file, const void *data, SceSize size) {
	u32 k1 = pspSdkSetK1(0);
	int level = sctrlKernelSetUserLevel(8);
	
	int ret = sceIoWrite(file, data, size);
	
	pspSdkSetK1(k1);
	sctrlKernelSetUserLevel(level);
	return ret;
}

int pspIoCloseFile(SceUID file) {
	u32 k1 = pspSdkSetK1(0);
	int level = sctrlKernelSetUserLevel(8);
	
	int ret = sceIoClose(file);
	
	pspSdkSetK1(k1);
	sctrlKernelSetUserLevel(level);
	return ret;
}

int pspIoLseek(SceUID file, SceOff offset, int whence) {
	u32 k1 = pspSdkSetK1(0);
	int level = sctrlKernelSetUserLevel(8);
	
	int ret = sceIoLseek(file, offset, whence);
	
	pspSdkSetK1(k1);
	sctrlKernelSetUserLevel(level);
	return ret;
}

int pspIoLseek32(SceUID file, SceOff offset, int whence) {
	u32 k1 = pspSdkSetK1(0);
	int level = sctrlKernelSetUserLevel(8);
	
	int ret = sceIoLseek32(file, offset, whence);
	
	pspSdkSetK1(k1);
	sctrlKernelSetUserLevel(level);
	return ret;
}

int pspIoGetstat(const char *file, SceIoStat *stat) {
	u32 k1 = pspSdkSetK1(0);
	int level = sctrlKernelSetUserLevel(8);
	
	int ret = sceIoGetstat(file, stat);
	
	pspSdkSetK1(k1);
	sctrlKernelSetUserLevel(level);
	return ret;
}

int pspIoRename(const char *oldname, const char *newname) {
	u32 k1 = pspSdkSetK1(0);
	int level = sctrlKernelSetUserLevel(8);
	
	int ret = sceIoRename(oldname, newname);
	
	pspSdkSetK1(k1);
	sctrlKernelSetUserLevel(level);
	return ret;
}

int pspIoRemoveFile(const char *file) {
	u32 k1 = pspSdkSetK1(0);
	int level = sctrlKernelSetUserLevel(8);
	
	int ret = sceIoRemove(file);
	
	pspSdkSetK1(k1);
	sctrlKernelSetUserLevel(level);
	return ret;
}

int pspIoDevctl(const char *dev, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	u32 k1 = pspSdkSetK1(0);
	int level = sctrlKernelSetUserLevel(8);
	
	int ret = sceIoDevctl(dev, cmd, indata, inlen, outdata, outlen);
	
	pspSdkSetK1(k1);
	sctrlKernelSetUserLevel(level);
	return ret;
}

int module_start(SceSize args, void *argp) {
	return 0;
}

int module_stop(void) {
	return 0;
}
