#ifndef CMFILEMANAGER_TEXTURES_H
#define CMFILEMANAGER_TEXTURES_H

#include <glib2d.h>

extern g2dTexture *icon_app, *icon_archive, *icon_audio, *icon_cd, *icon_dir, *icon_dir_dark, *icon_file, *icon_image, *icon_prx, \
          *icon_text, *icon_check, *icon_check_dark, *icon_uncheck, *icon_uncheck_dark, *icon_toggle_on, *icon_toggle_dark_on, \
          *icon_toggle_off, *icon_radio_off, *icon_radio_on, *icon_radio_dark_off, *icon_radio_dark_on, *icon_nav_drawer, *icon_back, \
          *options_dialog, *options_dialog_dark, *properties_dialog, *properties_dialog_dark, *dialog, *dialog_dark, \
          *battery_20, *battery_20_charging, *battery_30, *battery_30_charging, *battery_50, *battery_50_charging, \
          *battery_60, *battery_60_charging, *battery_80, *battery_80_charging, *battery_90, *battery_90_charging, \
          *battery_full, *battery_full_charging, *battery_low, *battery_unknown, *wifi_off, *wifi_on, *usb_icon, \
          *default_artwork, *default_artwork_blur, *btn_play, *btn_pause, *btn_rewind, *btn_forward, \
          *btn_repeat, *btn_shuffle, *btn_repeat_overlay, *btn_shuffle_overlay, \
          *bg_header, *icon_sd, *icon_secure, *icon_sd_dark, *icon_secure_dark, *ic_play_btn;


void Textures_Load(void);
void Textures_Free(void);

#endif
