#include <pspiofilemgr.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

int log_print(const char* format, ...) {
	va_list list;
	char string[1024] = {0};

	va_start(list, format);
	vsprintf(string, format, list);
	va_end(list);

	SceUID fd = 0;
	if (R_FAILED(fd = sceIoOpen("debug.log", 
		PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777))) 
		return fd;

	sceIoWrite(fd, string, strlen(string));
	sceIoClose(fd);

	return 0;
}
