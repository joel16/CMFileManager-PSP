	.set noreorder

#include "pspimport.s"

    IMPORT_START "ModuleMgrForKernel",0x00010000
    IMPORT_FUNC  "ModuleMgrForKernel",0xFFB9B760,sceKernelLoadModule
    IMPORT_FUNC  "ModuleMgrForKernel",0xE6BF3960,sceKernelStartModule
    IMPORT_FUNC  "ModuleMgrForKernel",0x0D053026,sceKernelUnloadModule
    IMPORT_FUNC  "ModuleMgrForKernel",0x32292450,_sceKernelLoadModuleWithApitype2
    IMPORT_FUNC  "ModuleMgrForKernel",0xE3CCC6EA,sceKernelLoadModule_620
    IMPORT_FUNC  "ModuleMgrForKernel",0xDF8FFFAB,sceKernelStartModule_620
    IMPORT_FUNC  "ModuleMgrForKernel",0x9CEB18C4,sceKernelUnloadModule_620
    IMPORT_FUNC  "ModuleMgrForKernel",0xB691CB9F,sceKernelLoadModuleWithApitype2_620
    IMPORT_FUNC  "ModuleMgrForKernel",0x939E4270,sceKernelLoadModule_660
    IMPORT_FUNC  "ModuleMgrForKernel",0x3FF74DF1,sceKernelStartModule_660
    IMPORT_FUNC  "ModuleMgrForKernel",0x387E3CA9,sceKernelUnloadModule_660
    IMPORT_FUNC  "ModuleMgrForKernel",0x2B7FC10D,sceKernelLoadModuleWithApitype2_660
    IMPORT_FUNC  "ModuleMgrForKernel",0x4E62C48A,sceKernelLoadModuleBufferForKernel
