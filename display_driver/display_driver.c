#include <pspsdk.h>
#include <pspdisplay_kernel.h>
#include <pspsysmem_kernel.h>

PSP_MODULE_INFO("display_driver", PSP_MODULE_KERNEL, 1, 2);
PSP_NO_CREATE_MAIN_THREAD();

int sceDisplayEnable(void);
int sceDisplayDisable(void);
int sceDisplayEnable371(void);
int sceDisplayDisable371(void);

int pspGetBrightness(int *brightness) {
    u32 k1 = pspSdkSetK1(0);
    sceDisplayGetBrightness(brightness, 0);
    pspSdkSetK1(k1);
    return 0;
}

int pspSetBrightness(int brightness) {
    u32 k1 = pspSdkSetK1(0);
    sceDisplaySetBrightness(brightness, 0);
    pspSdkSetK1(k1);
    return 0;
}

int pspDisplayEnable(void) {
    u32 k1 = pspSdkSetK1(0);
    
    int ret = 0;
    if (sceKernelDevkitVersion() < 0x03070110)
        ret = sceDisplayEnable();
    else
        ret = sceDisplayEnable371();
        
    pspSdkSetK1(k1);
    return ret;
}

int pspDisplayDisable(void) {
    u32 k1 = pspSdkSetK1(0);
    
    int ret = 0;
    if (sceKernelDevkitVersion() < 0x03070110)
        ret = sceDisplayDisable();
    else
        ret = sceDisplayDisable371();
    
    pspSdkSetK1(k1);
    return ret;
}

int module_start(SceSize args, void *argp) {
    return 0; 
}

int module_stop(void) {
    return 0; 
}
