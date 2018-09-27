#pragma once

extern bool config_dark_theme;
extern int config_sort_by;

int Config_Save(bool config_dark_theme, int config_sort_by);
int Config_Load(void);