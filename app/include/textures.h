#ifndef _CMFILEMANAGER_TEXTURES_H_
#define _CMFILEMANAGER_TEXTURES_H_

#include <glib2d.h>

constexpr int NUM_ICONS = 6;
constexpr int NUM_BATT_ICONS = 6;

extern g2dTexture *file_icons[NUM_ICONS], *icon_dir, *icon_dir_dark, *icon_check, *icon_check_dark, *icon_uncheck, \
    *icon_uncheck_dark, *icon_toggle_on, *icon_toggle_dark_on, *icon_toggle_off, *icon_radio_off, \
    *icon_radio_on, *icon_radio_dark_off, *icon_radio_dark_on, *icon_nav_drawer, *icon_back, \
    *options_dialog, *options_dialog_dark, *properties_dialog, *properties_dialog_dark, *dialog, *dialog_dark, \
    *battery_charging[NUM_BATT_ICONS], *battery[NUM_BATT_ICONS], *usb_icon, \
    *default_artwork, *default_artwork_blur, *btn_play, *btn_pause, *btn_rewind, *btn_forward, \
    *btn_repeat, *btn_shuffle, *btn_repeat_overlay, *btn_shuffle_overlay, \
    *bg_header, *icon_sd, *icon_secure, *icon_sd_dark, *icon_secure_dark, *ic_play_btn;

namespace Textures {
    void Load(void);
    void Free(void);
}

#endif
