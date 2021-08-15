#ifndef _CMFILEMANAGER_TEXTURES_H_
#define _CMFILEMANAGER_TEXTURES_H_

#include <glib2d.h>
#include <string>

constexpr int NUM_FILE_ICONS = 6;
constexpr int NUM_BATT_ICONS = 6;
constexpr int NUM_THEMES = 2;

extern g2dTexture *file_icons[NUM_FILE_ICONS], *icon_dir[NUM_THEMES], *icon_check[NUM_THEMES], *icon_uncheck[NUM_THEMES], \
    *icon_toggle_on[NUM_THEMES], *icon_toggle_off, *icon_radio_off[NUM_THEMES], *icon_radio_on[NUM_THEMES], *icon_nav_drawer, \
    *icon_back, *options_dialog[NUM_THEMES], *properties_dialog[NUM_THEMES], *dialog[NUM_THEMES], \
    *battery_charging[NUM_BATT_ICONS], *battery[NUM_BATT_ICONS], *usb_icon, \
    *default_artwork, *default_artwork_blur, *btn_play, *btn_pause, *btn_rewind, *btn_forward, \
    *btn_repeat, *btn_shuffle, *btn_repeat_overlay, *btn_shuffle_overlay, \
    *bg_header, *icon_sd[NUM_THEMES], *icon_secure[NUM_THEMES], *ic_play_btn, *ftp_icon[NUM_THEMES], *sort_icon[NUM_THEMES], \
    *dark_theme_icon[NUM_THEMES], *dev_options_icon[NUM_THEMES], *about_icon[NUM_THEMES];

namespace Textures {
    g2dTexture *LoadImageBufferJPEG(unsigned char *data, int size);
    g2dTexture *LoadImageBufferPNG(unsigned char *data, int size);
    g2dTexture *LoadImage(const std::string &path);
    void Load(void);
    void Free(void);
}

#endif
