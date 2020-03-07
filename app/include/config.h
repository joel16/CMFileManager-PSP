#pragma once

#include <stdbool.h>

typedef struct {
	int sort;
	bool dark_theme;
	bool auto_usb_mount;
	bool dev_options;
	bool large_icons;
} config_t;

config_t config;

int Config_Save(config_t config);
int Config_Load(void);
int Config_GetLastDirectory(void);
