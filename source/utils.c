#include <pspkernel.h>
#include <pspreg.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include <stdbool.h>
#include <stdio.h>

#include "common.h"
#include "config.h"
#include "log.h"
#include "kubridge.h"
#include "pspusbdevice.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "utils.h"

#define PATH_FLASH0 "flash0:/"
#define PATH_USBDEVICE PATH_FLASH0 "kd/_usbdevice.prx"
#define NELEMS(a) (sizeof(a) / sizeof(a[0]))

static SceCtrlData current, previous;
static bool g_usb_module_loaded = false;
static bool g_usb_actived = false;

struct UsbModule {
	char *path;
	int modid;
};

static struct UsbModule g_usb_modules[] = {
	{ PATH_USBDEVICE, -1, },
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
	SceIoDirent* entryA = (SceIoDirent*) p1;
	SceIoDirent* entryB = (SceIoDirent*) p2;
	
	if ((FIO_S_ISDIR(entryA->d_stat.st_mode)) && !(FIO_S_ISDIR(entryB->d_stat.st_mode)))
		return -1;
	else if (!(FIO_S_ISDIR(entryA->d_stat.st_mode)) && (FIO_S_ISDIR(entryB->d_stat.st_mode)))
		return 1;
		
	return strcasecmp(entryA->d_name, entryB->d_name);
}

static int Utils_LoadStartModule(char *path) {
	int ret = 0;
	SceUID modid = 0;

	modid = kuKernelLoadModule(path, 0, NULL);
	ret = sceKernelStartModule(modid, strlen(path) + 1, path, 0, NULL);

	return ret;
}

static void Utils_StopUnloadModules(SceUID modid) {
	sceKernelStopModule(modid, 0, NULL, NULL, NULL);
	sceKernelUnloadModule(modid);
}

/*void Utils_LoadUSBModules(void) {
	int i = 0;

	if (!g_usb_module_loaded) {
		for (i = 0; i < NELEMS(g_usb_modules); ++i) {
			g_usb_modules[i].modid = Utils_LoadStartModule(g_usb_modules[i].path);
		}

		g_usb_module_loaded = true;
	}
}

void Utils_UnloadUSBModules(void) {
	int i = 0;

	if (g_usb_module_loaded) {
		for (i = NELEMS(g_usb_modules) - 1; i >= 0; --i) {
			Utils_StopUnloadModules(g_usb_modules[i].modid);
			g_usb_modules[i].modid = -1;
		}

		g_usb_module_loaded = false;
	}
}*/

void Utils_InitUSB(void) {
	int i = 0;

	if (!g_usb_module_loaded) {
		for (i = 0; i < NELEMS(g_usb_modules); ++i) {
			g_usb_modules[i].modid = Utils_LoadStartModule(g_usb_modules[i].path);
		}

		g_usb_module_loaded = true;
	}

	sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
	sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
	sceUsbstorBootSetCapacity(0x800000);
	g_usb_actived = true;
}

static void Utils_StartUSBStorage(void) {
	sceUsbActivate(0x1c8);
	psp_usb_cable_connection = true;
}

static void Utils_StopUSBStorage(void) {
	sceUsbDeactivate(0x1c8);
	sceIoDevctl("fatms0:", 0x0240D81E, NULL, 0, NULL, 0 ); // Avoid corrupted files
	psp_usb_cable_connection = false;
}

static void Utils_DisableUSB(void) {
	if (!g_usb_actived)
		return;

	Utils_StopUSBStorage();
	sceUsbStop(PSP_USBSTOR_DRIVERNAME, 0, 0);
	sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
	pspUsbDeviceFinishDevice();
	g_usb_actived = false;
}

void Utils_ExitUSB(void) {
	int i = 0;

	Utils_DisableUSB();

	if (g_usb_module_loaded) {
		for (i = NELEMS(g_usb_modules) - 1; i >= 0; --i) {
			Utils_StopUnloadModules(g_usb_modules[i].modid);
			g_usb_modules[i].modid = -1;
		}

		g_usb_module_loaded = false;
	}
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

bool Utils_IsEF0(void) {
	if ((Utils_IsModelPSPGo()) && (kuKernelBootFrom() == 0x50))
		return true;
	else
		return false;
	
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

	if (R_FAILED(ret = sctrlKernelLoadExecVSHWithApitype(Utils_IsEF0()? 0x152 : 0x141, path, &param)))
		return ret;

	return 0;
}

int Utils_LaunchPOPS(const char *path) {
	int ret = 0;
	struct SceKernelLoadExecVSHParam param;
	memset(&param, 0, sizeof(param));

	param.size = sizeof(param);
	param.args = strlen(path) + 1;
	param.argp = (void *)path;
	param.key = "pops";

	if (R_FAILED(ret = sctrlKernelLoadExecVSHWithApitype(Utils_IsEF0()? 0x155 : 0x144, path, &param)))
		return ret;

	return 0;
}

int Utils_LaunchISO(const char *path) {
	int ret = 0;
	struct SceKernelLoadExecVSHParam param;
	memset(&param, 0, sizeof(param));

	param.size = sizeof(param);
	char *eboot_path = "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN";
	param.args = strlen(eboot_path) + 1;
	param.argp = eboot_path;
	param.key = "umdemu";

	sctrlSESetBootConfFileIndex(MODE_INFERNO);
	sctrlSESetUmdFile((char *)path);

	if (R_FAILED(ret = sctrlKernelLoadExecVSHWithApitype(Utils_IsEF0()? 0x125 : 0x123, path, &param)))
		return ret;

	return 0;
}

u64 Utils_GetTotalStorage(void) {
	int ret = 0;
	SystemDevCtl devctl;
	memset(&devctl, 0, sizeof(SystemDevCtl));
	SystemDevCommand command;
	command.pdevinf = &devctl;

	if (R_FAILED(ret = sceIoDevctl("ms0:", 0x02425818, &command, sizeof(SystemDevCommand), NULL, 0)))
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

	if (R_FAILED(ret = sceIoDevctl("ms0:", 0x02425818, &command, sizeof(SystemDevCommand), NULL, 0)))
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

void Utils_ReadControls(void) {
	previous = current;
	sceCtrlReadBufferPositive(&current, 1);
}

int Utils_IsButtonPressed(enum PspCtrlButtons buttons) {
	return (!(previous.Buttons & buttons)) && current.Buttons & buttons;
}

int Utils_IsButtonHeld(enum PspCtrlButtons buttons) {
	return current.Buttons & buttons;
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
