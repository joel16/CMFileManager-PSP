    .set noreorder

#include "pspstub.s"

    STUB_START "ThreadManForUser",0x40010000,0x00040005
    STUB_FUNC  0xB7D098C6,sceKernelCreateMutex
    STUB_FUNC  0xF8170FBE,sceKernelDeleteMutex
    STUB_FUNC  0xB011B11F,sceKernelLockMutex
    STUB_FUNC  0x6B30100F,sceKernelUnlockMutex
    STUB_END
