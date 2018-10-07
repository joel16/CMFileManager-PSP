#include <stdbool.h>

#include "common.h"
#include "config.h"

void Utils_EndDrawing(void) {
	oslEndDrawing();
	oslEndFrame();
	oslSyncFrame();
}

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

void Utils_DisplayKeyboard(char *descStr, char *initialStr, char *text) {
	int skip = 0;
	bool done = false;

	oslInitOsk(descStr, initialStr, 256, 1, -1);

	while(!osl_quit && !done) {
		if (!skip) {
			oslStartDrawing();
			oslDrawFillRect(0, 0, 480, 20, config_dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
			oslDrawFillRect(0, 20, 480, 62, config_dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
			if (oslOskIsActive())
				oslDrawOsk();
			if (oslGetOskStatus() == PSP_UTILITY_DIALOG_NONE) {
				if (oslOskGetResult() == OSL_OSK_CANCEL) {
					strcpy(text, "");
					done = 1;
				}
				else {
					oslOskGetText(text);
					done = 1;
				}
				oslEndOsk();
			}
			oslEndDrawing();
		}
		oslEndFrame();
		skip = oslSyncFrame();
	}
}