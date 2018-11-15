#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "utils.h"

#define LOG_BUFFER_LEN 4096

int log_print(const char* format, ...) {
	va_list list;
	char string[LOG_BUFFER_LEN] = {0};

	va_start(list, format);
	vsprintf(string, format, list);
	va_end(list);

	SceUID fd = 0;
	if (R_FAILED(fd = sceIoOpen(Utils_IsEF0()? "ef0:/PSP/GAME/CMFileManager/debug.log" : "ms0:/PSP/GAME/CMFileManager/debug.log", 
		PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777))) 
		return fd;

	sceIoWrite(fd, string, strlen(string));
	sceIoClose(fd);

	return 0;
}
