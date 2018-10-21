#include <stdbool.h>

#include "common.h"
#include "config.h"
#include "kubridge.h"

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
	char buf[100] = {}; // 100 so that it's big enough. fill with 0
	// or you could use malloc() to allocate sufficient space
	
	strncpy(buf, subject, pos); // copy at most first pos characters
	int len = strlen(buf);
	strcpy(buf+len, insert); // copy all of insert[] at the end
	len += strlen(insert);  // increase the length by length of insert[]
	strcpy(buf+len, subject+pos); // copy the rest
	
	strcpy(subject, buf);   // copy it back to subject
	// deallocate buf[] here, if used malloc()
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
