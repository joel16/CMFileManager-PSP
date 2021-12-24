#ifndef _CMFILEMANAGER_MUTEX_H_
#define _CMFILEMANAGER_MUTEX_H_

#define PSP_MUTEX_ATTR_FIFO 0

#ifdef __cplusplus
extern "C" {
#endif

int sceKernelCreateMutex(const char *name, uint attributes, int initial_count, void *options);
int sceKernelDeleteMutex(int mutexId);
int sceKernelLockMutex(int mutexId, int count, uint *timeout);
int sceKernelUnlockMutex(int mutexId, int count);

#ifdef __cplusplus
}
#endif


#endif
