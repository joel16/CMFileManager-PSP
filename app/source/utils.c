#include <pspkernel.h>
#include <pspreg.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include <stdio.h>

#include "common.h"
#include "config.h"
#include "kernel_functions.h"
#include "kubridge.h"
#include "log.h"
#include "pspusbdevice.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "utils.h"

#define NELEMS(a) (sizeof(a) / sizeof(a[0]))
#define CTRL_DELAY 100000
#define CTRL_DEADZONE_DELAY 500000

static SceCtrlData pad, prev_pad;
static int last_button = 0, last_button_tick = 0, deadzone_tick = 0;
static bool g_usb_module_loaded = false;
static bool g_usb_actived = false;

static SceUID kernel_driver[5];
const char *kernel_driver_path[] = {
	"audio_driver.prx",
	"display_driver.prx",
	"fs_driver.prx",
	"impose_driver.prx",
	"input_driver.prx"
};

struct UsbModule {
	const char *path;
	int modID;
};

static struct UsbModule g_usb_modules[] = {
	{ "flash0:/kd/_usbdevice.prx", -1, },
	{ "flash0:/kd/semawm.prx", -1, },
	{ "flash0:/kd/usbstor.prx", -1, },
	{ "flash0:/kd/usbstormgr.prx", -1, },
	{ "flash0:/kd/usbstorms.prx", -1, },
	{ "flash0:/kd/usbstoreflash.prx", -1, },
	{ "flash0:/kd/usbstorboot.prx", -1, },
};

typedef struct {
	unsigned long maxclusters;
	unsigned long freeclusters;
	int unk1;
	unsigned int sectorsize;
	u64 sectorcount;
} SystemDevCtl;

typedef struct {
	SystemDevCtl *pdevinf;    
} SystemDevCommand;

bool psp_usb_cable_connection = false;

void Utils_SetMax(int *set, int value, int max) {
	if (*set > max)
		*set = value;
}

void Utils_SetMin(int *set, int value, int min) {
	if (*set < min)
		*set = value;
}

char *Utils_Basename(const char *filename) {
	char *p = strrchr (filename, '/');
	return p ? p + 1 : (char *) filename;
}

void Utils_GetSizeString(char *string, u64 size) {
	double double_size = (double)size;

	int i = 0;
	static char *units[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

	while (double_size >= 1024.0f) {
		double_size /= 1024.0f;
		i++;
	}

	sprintf(string, "%.*f %s", (i == 0) ? 0 : 2, double_size, units[i]);
}

void Utils_AppendArr(char subject[], const char insert[], int pos) {
	char buf[512] = {};

	strncpy(buf, subject, pos);
	int len = strlen(buf);
	strcpy(buf + len, insert);
	len += strlen(insert);
	strcpy(buf+len, subject + pos);
	
	strcpy(subject, buf);
}

int Utils_Alphasort(const void *p1, const void *p2) {
	SceIoDirent *entryA = (SceIoDirent *)p1;
	SceIoDirent *entryB = (SceIoDirent *)p2;
	
	if ((FIO_S_ISDIR(entryA->d_stat.st_mode)) && !(FIO_S_ISDIR(entryB->d_stat.st_mode)))
		return -1;
	else if (!(FIO_S_ISDIR(entryA->d_stat.st_mode)) && (FIO_S_ISDIR(entryB->d_stat.st_mode)))
		return 1;
		
	return strcasecmp(entryA->d_name, entryB->d_name);
}

static int Utils_LoadStartModule(const char *path) {
	int ret = 0, status = 0;
	SceUID modID = 0;

	if (R_FAILED(ret = modID = kuKernelLoadModule(path, 0, NULL))) {
		Log_Print("kuKernelLoadModule(%s) failed: 0x%lx\n", path, ret);
		return ret;
	}
	
	if (R_FAILED(ret = sceKernelStartModule(modID, 0, NULL, &status, NULL))) {
		Log_Print("sceKernelStartModule(%s) failed: 0x%lx\n", path, ret);
		return ret;
	}

	return ret;
}

static void Utils_StopUnloadModules(SceUID modID) {
	sceKernelStopModule(modID, 0, NULL, NULL, NULL);
	sceKernelUnloadModule(modID);
}

static int Utils_InitUSB(void) {
	int i = 0, ret = 0;

	if (!g_usb_module_loaded) {
		for (i = 0; i < NELEMS(g_usb_modules); ++i) {
			g_usb_modules[i].modID = Utils_LoadStartModule(g_usb_modules[i].path);
		}

		g_usb_module_loaded = true;
	}

	if (R_FAILED(ret = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0))) {
		Log_Print("sceUsbStart(PSP_USBBUS_DRIVERNAME) failed: 0x%lx\n", ret);
		return ret;
	}
	
	if (R_FAILED(ret = sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0))) {
		Log_Print("sceUsbStart(PSP_USBSTOR_DRIVERNAME) failed: 0x%lx\n", ret);
		return ret;
	}

	if (R_FAILED(ret = sceUsbstorBootSetCapacity(0x800000))) {
		Log_Print("sceUsbstorBootSetCapacity(0x800000) failed: 0x%lx\n", ret);
		return ret;
	}
	
	g_usb_actived = true;
	return 0;
}

static int Utils_StartUSBStorage(void) {
	int ret = 0;
	
	if (R_FAILED(ret = sceUsbActivate(0x1c8))) {
		Log_Print("sceUsbActivate(0x1c8) failed: 0x%lx\n", ret);
		return ret;
	}
	
	psp_usb_cable_connection = true;
	return 0;
}

static int Utils_StopUSBStorage(void) {
	int ret = 0;

	if (R_FAILED(ret = sceUsbDeactivate(0x1c8))) {
		Log_Print("sceUsbActivate(0x1c8) failed: 0x%lx\n", ret);
		return ret;
	}
	
	if (R_FAILED(ret = sceIoDevctl("fatms0:", 0x0240D81E, NULL, 0, NULL, 0))) {// Avoid corrupted files
		Log_Print("sceIoDevctl(\"fatms0:\", 0x0240D81E, NULL, 0, NULL, 0) failed: 0x%lx\n", ret);
		return ret;
	}
	
	psp_usb_cable_connection = false;
	return 0;
}

static int Utils_DisableUSB(void) {
	int ret = 0;

	if (!g_usb_actived)
		return -1;

	if (R_FAILED(ret = Utils_StopUSBStorage()))
		return ret;
	
	if (R_FAILED(ret = sceUsbStop(PSP_USBSTOR_DRIVERNAME, 0, 0))) {
		Log_Print("sceUsbStop(PSP_USBSTOR_DRIVERNAME) failed: 0x%lx\n", ret);
		return ret;
	}
	
	if (R_FAILED(ret = sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0))) {
		Log_Print("sceUsbStop(PSP_USBBUS_DRIVERNAME) failed: 0x%lx\n", ret);
		return ret;
	}
	
	if (R_FAILED(ret = pspUsbDeviceFinishDevice())) {
		Log_Print("pspUsbDeviceFinishDevice() failed: 0x%lx\n", ret);
		return ret;
	}
	
	g_usb_actived = false;
	return 0;
}

static void Utils_ExitUSB(void) {
	int i = 0;

	Utils_DisableUSB();

	if (g_usb_module_loaded) {
		for (i = NELEMS(g_usb_modules) - 1; i >= 0; --i) {
			Utils_StopUnloadModules(g_usb_modules[i].modID);
			g_usb_modules[i].modID = -1;
		}

		g_usb_module_loaded = false;
	}
}

void Utils_InitKernelDrivers(void) {
	int i = 0;
	for (i = 0; i < 5; i++) {
		kernel_driver[i] = Utils_LoadStartModule(kernel_driver_path[i]);
	}
	
	Utils_InitUSB();
}

void Utils_TermKernelDrivers(void) {
	int i = 0;
	for (i = 0; i < 5; i++) {
		if (kernel_driver[i])
			Utils_StopUnloadModules(kernel_driver[i]);
	}

	Utils_ExitUSB();
}

void Utils_HandleUSB(void) {
	if (config.auto_usb_mount) {
		if (sceUsbGetState() & PSP_USB_CABLE_CONNECTED) {
			if (psp_usb_cable_connection == false)
				Utils_StartUSBStorage();
		}
		else {
			if (psp_usb_cable_connection == true)
				Utils_StopUSBStorage();
		}
	}
}

bool Utils_IsModelPSPGo(void) {
	if (kuKernelGetModel() == PSP_GO)
		return true;
	else
		return false;
	
	return false;
}

int Utils_IsMemCardInserted(bool *is_inserted) {
	int status = 0, ret = 0;
	if (R_FAILED(ret = sceIoDevctl("mscmhc0:", 0x02025806, 0, 0, &status, sizeof(status))))
		return ret;

	if (status != 1)
		*is_inserted = false;
	else
		*is_inserted = true;

	return 0;
}

bool Utils_IsEF0(void) {
	if (is_psp_go) {
		if (!is_ms_inserted)
			return true;

		return true;
	}

	return false;
}

int Utils_LaunchEboot(const char *path) {
	int ret = 0;
	struct SceKernelLoadExecVSHParam param;
	memset(&param, 0, sizeof(param));

	param.size = sizeof(param);
	param.args = strlen(path) + 1;
	param.argp = (void *)path;
	param.key = "game";

	if (R_FAILED(ret = sctrlKernelLoadExecVSHWithApitype(Utils_IsEF0()? 0x152 : 0x141, path, &param))) {
		Log_Print("sctrlKernelLoadExecVSHWithApitype(%x, %s) failed: 0x%lx\n", Utils_IsEF0()? 0x152 : 0x141, path, ret);
		return ret;
	}

	return 0;
}

u64 Utils_GetTotalStorage(void) {
	int ret = 0;
	SystemDevCtl devctl;
	memset(&devctl, 0, sizeof(SystemDevCtl));
	SystemDevCommand command;
	command.pdevinf = &devctl;

	if (R_FAILED(ret = sceIoDevctl(BROWSE_STATE == BROWSE_STATE_INTERNAL? "ef0": "ms0:", 0x02425818, &command, sizeof(SystemDevCommand), NULL, 0)))
		return 0;

	u64 size = (devctl.maxclusters * devctl.sectorcount) * devctl.sectorsize;
	return size;
}

static u64 Utils_GetFreeStorage(void) {
	int ret = 0;
	SystemDevCtl devctl;
	memset(&devctl, 0, sizeof(SystemDevCtl));
	SystemDevCommand command;
	command.pdevinf = &devctl;

	if (R_FAILED(ret = sceIoDevctl(BROWSE_STATE == BROWSE_STATE_INTERNAL? "ef0": "ms0:", 0x02425818, &command, sizeof(SystemDevCommand), NULL, 0)))
		return 0;

	u64 size = (devctl.freeclusters * devctl.sectorcount) * devctl.sectorsize; 
	return size;
}

u64 Utils_GetUsedStorage(void) {
	return (Utils_GetTotalStorage() - Utils_GetFreeStorage());
}

static int Utils_GetRegistryValue(const char *dir, const char *name, unsigned int *value) {
	int ret = 0;
	struct RegParam reg;
	REGHANDLE h;

	memset(&reg, 0, sizeof(reg));
	reg.regtype = 1;
	reg.namelen = strlen("/system");
	reg.unk2 = 1;
	reg.unk3 = 1;
	strcpy(reg.name, "/system");
	
	if (R_SUCCEEDED(sceRegOpenRegistry(&reg, 2, &h))) {
		REGHANDLE hd;
		
		if (R_SUCCEEDED(sceRegOpenCategory(h, dir, 2, &hd))) {
			REGHANDLE hk;
			unsigned int type, size;

			if (R_SUCCEEDED(sceRegGetKeyInfo(hd, name, &hk, &type, &size))) {
				if (!sceRegGetKeyValue(hd, hk, value, 4)) {
					ret = 1;
					sceRegFlushCategory(hd);
				}
			}

			sceRegCloseCategory(hd);
		}

		sceRegFlushRegistry(h);
		sceRegCloseRegistry(h);
	}

	return ret;
}

int Utils_ReadControls(void) {
	prev_pad = pad;
	pad = pspGetButtons();
	//sceCtrlReadBufferPositive(&pad, 1);
	
	if (pad.Buttons == last_button) {
		if (pad.TimeStamp - deadzone_tick < CTRL_DEADZONE_DELAY)
			return 0;
			
		if (pad.TimeStamp - last_button_tick < CTRL_DELAY)
			return 0;
			
		last_button_tick = pad.TimeStamp;
		return last_button;
	}
	
	last_button = pad.Buttons;
	deadzone_tick = last_button_tick = pad.TimeStamp;
	return last_button;
}

int Utils_IsButtonPressed(enum PspCtrlButtons buttons) {
	return ((pad.Buttons & buttons) == buttons) && ((prev_pad.Buttons & buttons) != buttons);
}

int Utils_IsButtonHeld(enum PspCtrlButtons buttons) {
	return pad.Buttons & buttons;
}

int Utils_GetEnterButton(void) {
	unsigned int button = 0;
	if (R_SUCCEEDED(Utils_GetRegistryValue("/CONFIG/SYSTEM/XMB", "button_assign", &button))) {
		if (button == 0)
			return 0x002000; // PSP_CTRL_CIRCLE
		else
			return 0x004000; // PSP_CTRL_CROSS
	}

	return 0x004000; // By default return PSP_CTRL_CROSS
}

// Basically the opposite of Utils_GetEnterButton();
int Utils_GetCancelButton(void) {
	unsigned int button = 0;
	if (R_SUCCEEDED(Utils_GetRegistryValue("/CONFIG/SYSTEM/XMB", "button_assign", &button))) {
		if (button == 0)
			return 0x004000; // PSP_CTRL_CROSS
		else
			return 0x002000; // PSP_CTRL_CIRCLE
	}

	return 0x002000; // By default return PSP_CTRL_CIRCLE
}

float Utils_GetAnalogX(void) {
	return (((float)pad.Lx - 122.5f) / 122.5f);
}

float Utils_GetAnalogY(void) {
	return (((float)pad.Ly - 122.5f) / 122.5f);
}
