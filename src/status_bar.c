#include <stdbool.h>
#include <time.h>
#include <oslib/oslib.h>

#include "common.h"

static char *Clock_GetCurrentTime(void) {
	static char buffer[10];

	time_t unix_time = time(0);
	struct tm* time_struct = gmtime((const time_t*)&unix_time);
	int hours = time_struct->tm_hour;
	int minutes = time_struct->tm_min;
	
	bool amOrPm = false;
	
	if (hours < 12)
		amOrPm = true;
	if (hours == 0)
		hours = 12;
	else if (hours > 12)
		hours = hours - 12;

	if ((hours >= 1) && (hours < 10))
		snprintf(buffer, 10, "%2i:%02i %s", hours, minutes, amOrPm ? "AM" : "PM");
	else
		snprintf(buffer, 10, "%2i:%02i %s", hours, minutes, amOrPm ? "AM" : "PM");

	return buffer;
}

void StatusBar_DisplayTime(void) {
	oslIntraFontSetStyle(font, 0.6f, WHITE, RGBA(0, 0, 0, 0), INTRAFONT_ALIGN_LEFT);
	int width = oslGetStringWidth(Clock_GetCurrentTime());
	oslDrawString(475 - width, (20 - (font->charHeight - 6)) / 2, Clock_GetCurrentTime());
}