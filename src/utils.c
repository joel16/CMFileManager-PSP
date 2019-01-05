#include <stdbool.h>

#include "common.h"
#include "config.h"
#include "kubridge.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "utils.h"

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

void Utils_HandleUSB(void) {
	if (config.auto_usb_mount) {
		if (oslGetUsbState() & PSP_USB_CABLE_CONNECTED) {
			if (psp_usb_cable_connection == false) {
				oslStartUsbStorage();
				psp_usb_cable_connection = true;
			}
		}
		else {
			psp_usb_cable_connection = false;
			oslStopUsbStorage();
		}
	}
}

bool Utils_IsEF0(void) {
	int init_apitype = kuKernelInitApitype();

	if (init_apitype == 0x125) // NP9660/ISO MODE
		return true;
	else if (init_apitype == 0x155) // POPS
		return true;
	else if (init_apitype == 0x151) // UPDATER
		return true;
	else if (init_apitype == 0x152) // GAME
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
