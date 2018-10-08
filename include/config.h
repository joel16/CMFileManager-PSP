#pragma once

typedef struct {
	bool dark_theme;
	int sort;
	bool auto_usb_mount;
} config_t;

config_t config;

int Config_Save(config_t config);
int Config_Load(void);
int Config_GetLastDirectory(void);
