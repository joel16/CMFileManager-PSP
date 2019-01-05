#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "config.h"
#include "fs.h"
#include "utils.h"

#define CONFIG_VERSION 0

const char *configFile =
	"config_ver = %d\n"
	"theme = %d\n"
	"sort = %d\n"
	"usb = %d\n";

static int config_version_holder = 0;

int Config_Save(config_t config) {
	int ret = 0;
	char *buf = malloc(64);

	int len = snprintf(buf, 64, configFile, CONFIG_VERSION, config.dark_theme, config.sort, config.auto_usb_mount);

	if (R_FAILED(ret = FS_WriteFile("config.cfg", buf, len))) {
		free(buf);
		return ret;
	}

	free(buf);
	return 0;
}

int Config_Load(void) {
	int ret = 0;

	if (!FS_FileExists("config.cfg")) {
		config.dark_theme = false;
		config.sort = 0;
		config.auto_usb_mount = true;
		return Config_Save(config);
	}

	u64 size = FS_GetFileSize("config.cfg");
	char *buf = malloc(size + 1);

	if (R_FAILED(ret = FS_ReadFile("config.cfg", buf, size))) {
		free(buf);
		return ret;
	}

	buf[size] = '\0';
	sscanf(buf, configFile, &config_version_holder, &config.dark_theme, &config.sort, &config.auto_usb_mount);
	free(buf);

	// Delete config file if config file is updated. This will rarely happen.
	if (config_version_holder  < CONFIG_VERSION) {
		sceIoRemove("config.cfg");
		config.dark_theme = false;
		config.sort = 0;
		config.auto_usb_mount = true;
		return Config_Save(config);
	}

	return 0;
}

int Config_GetLastDirectory(void) {
	int ret = 0;
	char *buf = malloc(256);
	strcpy(root_path, Utils_IsEF0()? "ef0:/" : "ms0:/");
	
	if (FS_FileExists("lastdir.txt")) {
		if (R_FAILED(ret = FS_ReadFile("lastdir.txt", buf, 256))) {
			free(buf);
			return ret;
		}

		char tempPath[256];
		sscanf(buf, "%[^\n]s", tempPath);
		
		if (FS_DirExists(tempPath)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
			strcpy(cwd, tempPath);
		else
			strcpy(cwd, Utils_IsEF0()? "ef0:/" : "ms0:/");
		
		free(buf);
	}
	else {
		int len = snprintf(buf, 256, Utils_IsEF0()? "ef0:/" : "ms0:/");

		if (R_FAILED(ret = FS_WriteFile("lastdir.txt", buf, len))) {
			free(buf);
			return ret;
		}

		strcpy(cwd, buf); // Set Start Path to "ms0:/" if lastDir.txt hasn't been created.
		free(buf);
	}

	return 0;
}
