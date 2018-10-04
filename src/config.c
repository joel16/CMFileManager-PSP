#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "fs.h"
#include "utils.h"

bool config_dark_theme;
int config_sort_by;

const char *configFile =
	"theme = %d\n"
	"sortBy = %d\n";

int Config_Save(bool config_dark_theme, int config_sort_by) {
	int ret = 0;
	char *buf = (char *)malloc(64);

	int len = snprintf(buf, 64, configFile, config_dark_theme, config_sort_by);

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
		config_dark_theme = false;
		config_sort_by = 0;
		return Config_Save(config_dark_theme, config_sort_by);
	}

	u64 size = FS_GetFileSize("config.cfg");
	char *buf = (char *)malloc(size + 1);

	if (R_FAILED(ret = FS_ReadFile("config.cfg", buf, 128))) {
		free(buf);
		return ret;
	}

	buf[size] = '\0';
	sscanf(buf, configFile, &config_dark_theme, &config_sort_by);
	free(buf);

	return 0;
}
