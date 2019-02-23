#include <psppower.h>
#include <psprtc.h>
#include <pspnet_apctl.h>
#include <stdio.h>

#include "common.h"
#include "glib2d_helper.h"
#include "textures.h"
#include "utils.h"

static int percent_width = 0;

bool IsWlanConnected(void) {
	union SceNetApctlInfo apctlInfo;

	if (R_SUCCEEDED(sceNetApctlGetInfo(PSP_NET_APCTL_INFO_IP, &apctlInfo)))
		return true;

	return false;
}

static char *Clock_GetCurrentTime(void) {
	static char buffer[15];
	pspTime time;

	if (R_SUCCEEDED(sceRtcGetCurrentClockLocalTime(&time))) {
		bool amOrPm = false;
	
		if (time.hour < 12)
			amOrPm = true;
		if (time.hour == 0)
			time.hour = 12;
		else if (time.hour > 12)
			time.hour = time.hour - 12;

		if ((time.hour >= 1) && (time.hour < 10))
			snprintf(buffer, 15, "%2i:%02i %s", time.hour, time.minutes, amOrPm ? "AM" : "PM");
		else
			snprintf(buffer, 15, "%2i:%02i %s", time.hour, time.minutes, amOrPm ? "AM" : "PM");
	}

	return buffer;
}

static void StatusBar_GetBatteryStatus(int x, int y) {
	int percent = 0, state = 0;
	char buf[13];

	if (R_FAILED(state = scePowerIsBatteryCharging()))
		state = 0;

	if (R_SUCCEEDED(percent = scePowerGetBatteryLifePercent())) {
		if (percent < 20)
			G2D_DrawImage(battery_low, x, 3);
		else if ((percent >= 20) && (percent < 30)) {
			if (state != 0)
				G2D_DrawImage(battery_20_charging, x, 2);
			else
				G2D_DrawImage(battery_20, x, 2);
		}
		else if ((percent >= 30) && (percent < 50)) {
			if (state != 0)
				G2D_DrawImage(battery_50_charging, x, 2);
			else
				G2D_DrawImage(battery_50, x, 2);
		}
		else if ((percent >= 50) && (percent < 60)) {
			if (state != 0)
				G2D_DrawImage(battery_50_charging, x, 2);
			else
				G2D_DrawImage(battery_50, x, 2);
		}
		else if ((percent >= 60) && (percent < 80)) {
			if (state != 0)
				G2D_DrawImage(battery_60_charging, x, 2);
			else
				G2D_DrawImage(battery_60, x, 2);
		}
		else if ((percent >= 80) && (percent < 90)) {
			if (state != 0)
				G2D_DrawImage(battery_80_charging, x, 2);
			else
				G2D_DrawImage(battery_80, x, 2);
		}
		else if ((percent >= 90) && (percent < 100)) {
			if (state != 0)
				G2D_DrawImage(battery_90_charging, x, 2);
			else
				G2D_DrawImage(battery_90, x, 2);
		}
		else if (percent == 100) {
			if (state != 0)
				G2D_DrawImage(battery_full_charging, x, 2);
			else
				G2D_DrawImage(battery_full, x, 2);
		}

		snprintf(buf, 13, "%d%%", percent);
		percent_width = intraFontMeasureText(font, buf); 
		intraFontPrint(font, (x - percent_width - 6), y, buf);
	}
	else {
		snprintf(buf, 13, "%d%%", percent);
		percent_width = intraFontMeasureText(font, buf);
		intraFontPrint(font, (x - percent_width - 6), y, buf);
		G2D_DrawImage(battery_unknown, x, 2);
	}
}

void StatusBar_DisplayTime(void) {
	if (psp_usb_cable_connection)
		G2D_DrawImage(usb_icon, 0, 0);

	intraFontSetStyle(font, 0.7f, WHITE, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
	int width = intraFontMeasureText(font, Clock_GetCurrentTime());
	IsWlanConnected()? G2D_DrawImage(wifi_on, 475 - width - 22 - (percent_width + 6) - 22, 2) : G2D_DrawImage(wifi_off, 475 - width - 22 - (percent_width + 6) - 22, 2);
	StatusBar_GetBatteryStatus(475 - width - 22, ((20 - ((font->texYSize - 30))) / 2) + 2);
	intraFontPrint(font, 475 - width, ((20 - ((font->texYSize - 30))) / 2) + 2, Clock_GetCurrentTime());
}
