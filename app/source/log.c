#include <pspiofilemgr.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "utils.h"

static SceUID log_handle = 0;

int Log_OpenFileHande(void) {
	int ret = 0;

	if (R_FAILED(ret = log_handle = sceIoOpen("debug.log", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777)))
		return ret;

	return 0;
}

int Log_CloseFileHandle(void) {
	int ret = 0;

	if (R_FAILED(ret = sceIoClose(log_handle)))
		return ret;

	return 0;
}

int Log_Print(const char *format, ...) {
	if (!config.dev_options)
		return -1;
	
	va_list list;
	char string[1024] = {0};

	va_start(list, format);
	vsprintf(string, format, list);
	va_end(list);

	int ret = 0;
	if (R_FAILED(ret = sceIoWrite(log_handle, string, strlen(string))))
		return ret;

	return 0;
}
