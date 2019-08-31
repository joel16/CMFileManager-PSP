#include <pspdisplay.h>
#include <pspgu.h>
#include <pspkernel.h>
#include <pspnet.h>
#include <pspnet_apctl.h>
#include <pspnet_inet.h>
#include <psppower.h>
#include <psputility.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "ftppsp.h"
#include "glib2d_helper.h"
#include "screenshot.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

static int running = 1;

int FTP_DisplayNetDialog(void) {
	running = 1;
	int done = 0;

   	pspUtilityNetconfData data;

	memset(&data, 0, sizeof(data));
	data.base.size = sizeof(data);
	data.base.language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
	data.base.buttonSwap = PSP_UTILITY_ACCEPT_CROSS;
	data.base.graphicsThread = 17;
	data.base.accessThread = 19;
	data.base.fontThread = 18;
	data.base.soundThread = 16;
	data.action = PSP_NETCONF_ACTION_CONNECTAP;
	
	struct pspUtilityNetconfAdhoc adhocparam;
	memset(&adhocparam, 0, sizeof(adhocparam));
	data.adhocparam = &adhocparam;

	sceUtilityNetconfInitStart(&data);
	
	while(running) {
		g2dClear(BLACK_BG);
		g2dFlip(G2D_VSYNC);

		switch(sceUtilityNetconfGetStatus()) {
			case PSP_UTILITY_DIALOG_NONE:
				running = 1;
				break;

			case PSP_UTILITY_DIALOG_VISIBLE:
				sceUtilityNetconfUpdate(1);
				running = 1;
				break;

			case PSP_UTILITY_DIALOG_QUIT:
				sceUtilityNetconfShutdownStart();
				//running = 0;
				break;
				
			case PSP_UTILITY_DIALOG_FINISHED:
				done = 1;
				break;

			default:
				break;
		}
		
		if (done) {
			running = 1;
			break;
		}
	}
	
	return 1;
}

static int FTP_NetInit(void) {
	int ret = 0;

	if (R_FAILED(ret = sceNetInit(128 * 1024, 42, 4 * 1024, 42, 4 * 1024)))
		return ret;

	if (R_FAILED(ret = sceNetInetInit()))
		return ret;

	if (R_FAILED(ret = sceNetApctlInit(0x8000, 48)))
		return ret;

	return 0;
}

void FTP_NetTerm(void) {
	sceNetApctlTerm();
	sceNetInetTerm();
	sceNetTerm();
}

void Menu_DisplayFTP(void) {
	char psp_ip[16];
	unsigned short int psp_port;
	int ret = 0;

	scePowerLock(0);
	sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	sceUtilityLoadNetModule(PSP_NET_MODULE_INET);

	if (R_FAILED(FTP_NetInit()))
		return;

	FTP_DisplayNetDialog();

	ret = ftppsp_init(psp_ip, &psp_port);

	if (is_psp_go) {
		if (is_ms_inserted) {
			ftppsp_add_device("ms0:");
			ftppsp_add_device("ef0:");
		}
		else
			ftppsp_add_device("ef0:");
	}
	else
		ftppsp_add_device("ms0:");

	char *msg = malloc(64);

	if (ret < 0)
		snprintf(msg, 64, "Connection Failed. Please enable Wi-Fi");
	else
		snprintf(msg, 64, "FTP Connection established %s:%i", psp_ip, psp_port);

	int msg_width = intraFontMeasureText(font, msg);
	int result_width = intraFontMeasureText(font, "Press Cross/Circle to exit.");

	bool screen_disabled = false;

	while (1) {
		g2dClear(config.dark_theme? BLACK_BG : WHITE);
		G2D_DrawRect(0, 0, 480, 20, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
		G2D_DrawRect(0, 20, 480, 42, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
		G2D_DrawImage(icon_nav_drawer, 5, 25);

		StatusBar_DisplayTime();
		Dirbrowse_DisplayFiles();

		G2D_DrawRect(0, 20, 480, 252, G2D_RGBA(0, 0, 0, config.dark_theme? 50: 80));

		G2D_DrawImage(config.dark_theme? dialog_dark : dialog, ((480 - dialog->w) / 2), ((272 - dialog->h) / 2));

		intraFontSetStyle(font, 0.7f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
		intraFontPrint(font, ((480 - dialog->w) / 2) + 10, ((272 - dialog->h) / 2) + 20, "FTP");

		intraFontSetStyle(font, 0.7f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
		intraFontPrint(font, ((480 - (msg_width)) / 2), ((272 - dialog->h) / 2) + 50, msg);

		intraFontPrint(font, ((480 - (result_width)) / 2), ((272 - dialog->h) / 2) + 50 + 16, "Press Cross/Circle to exit.");

		intraFontSetStyle(font, 0.7f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
		G2D_DrawRect((409 - (intraFontMeasureText(font, "EXIT"))) - 5, (180 - (font->texYSize - 20)) - 5, intraFontMeasureText(font, "EXIT") + 10, (font->texYSize - 10) + 10, 
			config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
		intraFontPrint(font, 409 - (intraFontMeasureText(font, "EXIT")), (182 - (font->texYSize - 30)), "EXIT");

		g2dFlip(G2D_VSYNC);

		Utils_ReadControls();

		if (((Utils_IsButtonHeld(PSP_CTRL_LTRIGGER)) && (Utils_IsButtonPressed(PSP_CTRL_RTRIGGER))) || ((Utils_IsButtonHeld(PSP_CTRL_RTRIGGER)) && (Utils_IsButtonPressed(PSP_CTRL_LTRIGGER))))
			Screenshot_Capture();

		if (Utils_IsButtonPressed(PSP_CTRL_START)) {
			screen_disabled = !screen_disabled;

			if (screen_disabled)
				pspDisplayDisable();
			else
				pspDisplayEnable();
		}

		if ((Utils_IsButtonPressed(PSP_CTRL_ENTER)) || (Utils_IsButtonPressed(PSP_CTRL_CANCEL))) {
			sceKernelDelayThread(100 * 1000);
			break;
		}
	}

	free(msg);
	ftppsp_fini();
	FTP_NetTerm();

	sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
	sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
	scePowerUnlock(0);
	
	// If user tries to exit with screen disabled, enable it.
	if (screen_disabled)
		pspDisplayEnable();
}
