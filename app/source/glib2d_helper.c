#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspgu.h>
#include <string.h>
#include <psputility.h>
#include <malloc.h>

#include <glib2d.h>

#include "utils.h"

void G2D_DrawRect(float x, float y, float width, float height, g2dColor color) {
	g2dBeginRects(NULL); {
		g2dSetColor(color);
		g2dSetScaleWH(width, height);
		g2dSetCoordXY(x, y);
		g2dAdd();
	}
	g2dEnd();
}

void G2D_DrawImage(g2dTexture *tex, float x, float y) {
	g2dBeginRects(tex); {
		g2dSetCoordXY(x, y);
		g2dAdd();
	}
	g2dEnd();
}

void G2D_DrawImageScale(g2dTexture *tex, float x, float y, float w, float h) {
	g2dBeginRects(tex); {
		g2dSetScaleWH(w, h);
		g2dSetCoordXY(x, y);
		g2dAdd();
	}
	g2dEnd();
}

static int G2D_GetText(char *input, unsigned short *intext, unsigned short *desc){
	bool done = false;
	unsigned short outtext[128] = { 0 };

	SceUtilityOskData data;
	memset(&data, 0, sizeof(SceUtilityOskData));
	data.language = PSP_UTILITY_OSK_LANGUAGE_DEFAULT; // Use system default for text input
	data.lines = 1;
	data.unk_24 = 1;
	data.inputtype = PSP_UTILITY_OSK_INPUTTYPE_ALL; // Allow all input types
	data.desc = desc;
	data.intext = intext;
	data.outtextlength = 128;
	data.outtextlimit = 32; // Limit input to 32 characters
	data.outtext = outtext;

	SceUtilityOskParams params;
	memset(&params, 0, sizeof(params));
	params.base.size = sizeof(params);
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &params.base.language);
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &params.base.buttonSwap);
	params.base.graphicsThread = 17;
	params.base.accessThread = 19;
	params.base.fontThread = 18;
	params.base.soundThread = 16;
	params.datacount = 1;
	params.data = &data;

	int ret = 0;
	if (R_FAILED(ret = sceUtilityOskInitStart(&params)))
		return -1;

	while(!done) {
		int i = 0, j = 0;

		g2dClear(G2D_RGBA(39, 50, 56, 255));

		sceGuFinish();
		sceGuSync(0, 0);

		switch(sceUtilityOskGetStatus()) {
			case PSP_UTILITY_DIALOG_INIT:
				break;
			
			case PSP_UTILITY_DIALOG_VISIBLE:
				sceUtilityOskUpdate(1);
				break;
			
			case PSP_UTILITY_DIALOG_QUIT:
				sceUtilityOskShutdownStart();
				break;
			
			case PSP_UTILITY_DIALOG_FINISHED:
				break;
				
			case PSP_UTILITY_DIALOG_NONE:
				done = true;
				
			default :
				break;
		}

		for(i = 0; data.outtext[i]; i++) {
			if (data.outtext[i] != '\0' && data.outtext[i] != '\n' && data.outtext[i] != '\r') {
				input[j] = data.outtext[i];
				j++;
			}
		}
		
		input[j] = 0;
		g2dFlip(G2D_VSYNC);
	}

	return 1;
}

char *G2D_KeyboardGetText(char *desc_msg, char *initial_msg) {
	int ret = 0, i = 0;
	static char str[64];
	unsigned short initial[128]  = { 0 };
	unsigned short desc[128]  = { 0 };

	if (initial_msg[0] != 0) {
		for (i = 0; i <= strlen(initial_msg); i++)
			initial[i] = (unsigned short)initial_msg[i];
	}

	if (desc_msg[0] != 0) {
		for (i = 0; i <= strlen(desc_msg); i++)
			desc[i] = (unsigned short)desc_msg[i];
	}

	ret = G2D_GetText(str, initial, desc);

	if (ret)
		return str;

	return 0;
}
