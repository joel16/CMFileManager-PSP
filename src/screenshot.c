#include <oslib/oslib.h>
#include <psprtc.h>

#include "common.h"
#include "fs.h"
#include "utils.h"

static int num = 0;

static int Screenshot_GenFilename(int number, char *filename) {
	int ret = 0;
	pspTime time;

	if (R_FAILED(ret = sceRtcGetCurrentClockLocalTime(&time)))
		return ret;

	if (!(FS_DirExists(Utils_IsEF0()? "ef0:/PSP/PHOTO/CMFileManager/" : "ms0:/PSP/PHOTO/CMFileManager/")))
		FS_MakeDir(Utils_IsEF0()? "ef0:/PSP/PHOTO/CMFileManager" : "ms0:/PSP/PHOTO/CMFileManager");
	
	sprintf(filename, Utils_IsEF0()? "ef0:/PSP/PHOTO/CMFileManager/screenshot_%02d%02d%02d-%i.png" : 
		"ms0:/PSP/PHOTO/CMFileManager/screenshot_%02d%02d%02d-%i.png", time.year, time.month, time.day, num);

	return 0;
}

void Screenshot_Capture(void) {
	if ((BROWSE_STATE == BROWSE_STATE_INTERNAL) || (BROWSE_STATE == BROWSE_STATE_SD)) {
		static char filename[256];

		sprintf(filename, "%s", "screenshot");
		Screenshot_GenFilename(num, filename);

		while (FS_FileExists(filename)) {
			num++;
			Screenshot_GenFilename(num, filename);
		}

		oslWriteImageFilePNG(OSL_SECONDARY_BUFFER, filename, 0);
		num++;
	}
}