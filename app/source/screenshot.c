#include <pspdisplay.h>
#include <psptypes.h>
#include <sys/time.h>
#include <psprtc.h>
#include <malloc.h>
#include <stdio.h>

#include "common.h"
#include "fs.h"
#include "utils.h"

#define BMP_ID "BM"

struct BitmapHeader {
	char id[2];
	u32 filesize;
	u32 reserved;
	u32 offset;
	u32 headsize;
	u32 width;
	u32 height;
	u16 planes;
	u16 bpp;
	u32 comp;
	u32 bitmapsize;
	u32 hres;
	u32 vres;
	u32 colors;
	u32 impcolors;
} __attribute__((packed));

static int fixed_write(SceUID fd, void *data, int length) {
	int bytes_written = 0;

	while(bytes_written < length) {
		int ret;

		if (R_FAILED(ret = sceIoWrite(fd, data + bytes_written, length - bytes_written))) {
			bytes_written = -1;
			break;
		}

		bytes_written += ret;
	}

	return bytes_written;
}

static int write_8888_data(void *frame, void *pixel_data) {
	u8 *line;
	u8 *p;
	int i = 0, h = 0;

	line = pixel_data;

	for(h = 271; h >= 0; h--) {
		p = frame + (h * 512 * 4);

		for(i = 0; i < 480; i++) {
			line[(i * 3) + 2] = p[i * 4];
			line[(i * 3) + 1] = p[(i * 4) + 1];
			line[(i * 3) + 0] = p[(i * 4) + 2];
		}
		
		line += 480 * 3;
	}

	return 0;
}

static int bitmapWrite(void *frame_addr, void *tmp_buf, const char *file) {
	struct BitmapHeader *bmp;
	void *pixel_data = tmp_buf + sizeof(struct BitmapHeader);
	SceUID fd;

	bmp = (struct BitmapHeader *) tmp_buf;
	memset(bmp, 0, sizeof(struct BitmapHeader));
	memcpy(bmp->id, BMP_ID, sizeof(bmp->id));
	bmp->filesize = 480*272*3 + sizeof(struct BitmapHeader);
	bmp->offset = sizeof(struct BitmapHeader);
	bmp->headsize = 0x28;
	bmp->width = 480;
	bmp->height = 272;
	bmp->planes = 1;
	bmp->bpp = 24;
	bmp->bitmapsize = 480 * 272 * 3;
	bmp->hres = 2834;
	bmp->vres = 2834;

	write_8888_data(frame_addr, pixel_data);

	if (R_SUCCEEDED(fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777))) {
		fixed_write(fd, tmp_buf, bmp->filesize);
		sceIoClose(fd);
		return 0;
	}

	return -1;
}

static int G2D_WriteImageFilePNG(const char *path) {
	int ret = 0;
	void *mem = NULL;

	mem = malloc(391734);
	if (R_FAILED(ret = bitmapWrite(g2d_disp_buffer.data, mem, path))) {
		free(mem);
		return ret;
	}
	
	free(mem);
	return 0;
}

static int Screenshot_GenFilename(int count, char *filename) {
	int ret = 0;
	pspTime time;

	if (R_FAILED(ret = sceRtcGetCurrentClockLocalTime(&time)))
		return ret;

	if (!(FS_DirExists(Utils_IsEF0()? "ef0:/PSP/PHOTO/CMFileManager/" : "ms0:/PSP/PHOTO/CMFileManager/")))
		FS_RecursiveMakeDir(Utils_IsEF0()? "ef0:/PSP/PHOTO/CMFileManager" : "ms0:/PSP/PHOTO/CMFileManager");
	
	sprintf(filename, Utils_IsEF0()? "ef0:/PSP/PHOTO/CMFileManager/screenshot_%02d%02d%02d-%i.bmp" : 
		"ms0:/PSP/PHOTO/CMFileManager/screenshot_%02d%02d%02d-%i.bmp", time.year, time.month, time.day, count);

	return 0;
}

void Screenshot_Capture(void) {
	int count = 0;
	static char filename[256];

	sprintf(filename, "%s", "screenshot");
	Screenshot_GenFilename(count, filename);

	while (FS_FileExists(filename)) {
		count++;
		Screenshot_GenFilename(count, filename);
	}

	G2D_WriteImageFilePNG(filename);
	count++;
}
