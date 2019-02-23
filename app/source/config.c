#include <malloc.h>
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
	char *buf = NULL;
	buf = malloc(256);
	
	if (FS_FileExists("lastdir.txt")) {
		if (R_FAILED(ret = FS_ReadFile("lastdir.txt", buf, 256))) {
			free(buf);
			return ret;
		}

		char temp_path[256], drive[7];
		sscanf(buf, "%[^\n]s\n", temp_path);
		snprintf(drive, 7, "%.5s", temp_path);
		snprintf(root_path, 7, "%.5s", drive);

		if (!strcmp(root_path, "ef0:/"))
			BROWSE_STATE = BROWSE_STATE_INTERNAL;
		else
			BROWSE_STATE = BROWSE_STATE_SD;
		
		if (FS_DirExists(temp_path) && (!strcmp(drive, root_path))) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
			strcpy(cwd, temp_path);
		else
			strcpy(cwd, is_psp_go? "ef0:/" : "ms0:/");
		
		free(buf);
	}
	else {
		strcpy(root_path, is_psp_go? "ef0:/" : "ms0:/");
		int len = snprintf(buf, 256, is_psp_go? "ef0:/" : "ms0:/");

		if (is_psp_go)
			BROWSE_STATE = BROWSE_STATE_INTERNAL;

		if (R_FAILED(ret = FS_WriteFile("lastdir.txt", buf, len))) {
			free(buf);
			return ret;
		}

		strcpy(cwd, buf); // Set Start Path to "ms0:/" if lastDir.txt hasn't been created.
		free(buf);
	}

	return 0;
}
