#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "config.h"
#include "fs.h"
#include "jsmn.h"
#include "log.h"
#include "utils.h"

#define CONFIG_VERSION 2

const char *config_file = "{\n\
	\t\"config_ver\": %d,\n\
	\t\"sort\": %d,\n\
	\t\"dark_theme\": %d,\n\
	\t\"auto_usb\": %d,\n\
	\t\"dev_options\": %d,\n\
	\t\"large_icons\": %d\n}\n";
static int config_version_holder = 0;

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start && strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	
	return -1;
}

int Config_Save(config_t config) {
	int ret = 0;
	char *buf = (char *)calloc(128, sizeof(char));
	int len = snprintf(buf, 128, config_file, CONFIG_VERSION, config.sort, config.dark_theme, config.auto_usb_mount, config.dev_options, config.large_icons);

	if (R_FAILED(ret = FS_WriteFile("config.json", buf, len))) {
		Log_Print("Read config failed in Config_Save 0x%lx\n", ret);
		free(buf);
		return ret;
	}

	free(buf);
	return 0;
}

int Config_Load(void) {
	int ret = 0;

	// Set root path and current working directory based on model.
	strncpy(root_path, is_psp_go? "ef0:/" : "ms0:/", 6);
	strncpy(cwd, is_psp_go? "ef0:/" : "ms0:/", 6);

	if (!FS_FileExists("config.json")) {
		config.sort = 0;
		config.dark_theme = false;
		config.auto_usb_mount = true;
		config.dev_options = false;
		config.large_icons = true;
		Log_Print("File doesn't exist\n");
		return Config_Save(config);
	}

	u64 size = FS_GetFileSize("config.json");
	char *buf =  (char *)calloc(size + 1, sizeof(char));

	if (R_FAILED(ret = FS_ReadFile("config.json", buf, size))) {
		Log_Print("Read config failed in Config_Load 0x%lx\n", ret);
		free(buf);
		return ret;
	}

	buf[size] = '\0';
	jsmn_parser parser;
	jsmntok_t token[128];

	jsmn_init(&parser);
	if (R_FAILED(ret = jsmn_parse(&parser, buf, strlen(buf), token, 128))) {
		Log_Print("jsmn_parse failed in Config_Load %d\n", ret);
		return ret;
	}
	
	if (ret < 1 || token[0].type != JSMN_OBJECT) {
		Log_Print("jsmn_parse failed: object expected\n");
		return ret;
	}

	int i = 0;
	for (i = 1; i < ret; i++) {
		if (jsoneq(buf, &token[i], "config_ver") == 0) {
			config_version_holder = strtol(buf + token[i + 1].start, NULL, 10);
			i++;
		}
		else if (jsoneq(buf, &token[i], "sort") == 0) {
			config.sort = strtol(buf + token[i + 1].start, NULL, 10);
			i++;
		}
		else if (jsoneq(buf, &token[i], "dark_theme") == 0) {
			config.dark_theme = strtol(buf + token[i + 1].start, NULL, 10);
			i++;
		}
		else if (jsoneq(buf, &token[i], "auto_usb") == 0) {
			config.auto_usb_mount = strtol(buf + token[i + 1].start, NULL, 10);
			i++;
		}
		else if (jsoneq(buf, &token[i], "dev_options") == 0) {
			config.dev_options = strtol(buf + token[i + 1].start, NULL, 10);
			i++;
		}
		else if (jsoneq(buf, &token[i], "large_icons") == 0) {
			config.large_icons = strtol(buf + token[i + 1].start, NULL, 10);
			i++;
		}
		else
			Log_Print("Unexpected key: %.*s\n", token[i].end - token[i].start, buf + token[i].start);
	}

	free(buf);

	// Delete config file if config file is updated. This will rarely happen.
	if (config_version_holder  < CONFIG_VERSION) {
		sceIoRemove("config.json");
		config.sort = 0;
		config.dark_theme = false;
		config.auto_usb_mount = true;
		config.dev_options = false;
		config.large_icons = true;
		return Config_Save(config);
	}

	return 0;
}
