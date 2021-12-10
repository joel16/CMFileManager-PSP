#include <pspsdk.h>
#include <pspdisplay_kernel.h>
#include <pspsysmem_kernel.h>

PSP_MODULE_INFO("display_driver", PSP_MODULE_KERNEL, 1, 5);
PSP_NO_CREATE_MAIN_THREAD();

int sceDisplayEnable(void);
int sceDisplayDisable(void);
int sceDisplayEnable371(void);
int sceDisplayDisable371(void);
void sceDisplaySetBrightness371(int level, int unk1);
void sceDisplayGetBrightness371(int *level, int *unk1);

static int old_brightness_level = 0;

int pspDisplayEnable(void) {
    u32 k1 = pspSdkSetK1(0);
    
    int ret = 0;
    if (sceKernelDevkitVersion() < 0x03070110) {
        sceDisplaySetBrightness(old_brightness_level, 0);
        ret = sceDisplayEnable();
    }
    else {
        sceDisplaySetBrightness371(old_brightness_level, 0);
        ret = sceDisplayEnable371();
    }
        
    pspSdkSetK1(k1);
    return ret;
}

int pspDisplayDisable(void) {
    u32 k1 = pspSdkSetK1(0);
    
    int ret = 0;
    if (sceKernelDevkitVersion() < 0x03070110) {
        sceDisplaySetBrightness(0, 0);
        ret = sceDisplayDisable();
    }
    else {
        sceDisplaySetBrightness371(0, 0);
        ret = sceDisplayDisable371();
    }

    pspSdkSetK1(k1);
    return ret;
}

int module_start(SceSize args, void *argp) {
    int unk = 0;
    sceDisplayGetBrightness(&old_brightness_level, &unk);
    return 0;
}

int module_stop(void) {
    return 0;
}
