#include "textures.h"

extern unsigned char ic_fso_type_app_png_start[], ic_fso_type_compress_png_start[], ic_fso_type_audio_png_start[], 
    ic_fso_folder_png_start[], ic_fso_folder_dark_png_start[], ic_fso_default_png_start[], ic_fso_type_image_png_start[], 
    ic_fso_type_text_png_start[], btn_material_light_check_on_normal_png_start[], btn_material_light_check_on_normal_dark_png_start[], 
    btn_material_light_check_off_normal_png_start[], btn_material_light_check_off_normal_dark_png_start[], btn_material_light_toggle_on_normal_png_start[], 
    btn_material_light_toggle_on_normal_dark_png_start[], btn_material_light_toggle_off_normal_png_start[], btn_material_light_radio_off_normal_png_start[], 
    btn_material_light_radio_on_normal_png_start[], btn_material_light_radio_off_normal_dark_png_start[], btn_material_light_radio_on_normal_dark_png_start[], 
    ic_material_light_navigation_drawer_png_start[], ic_arrow_back_normal_png_start[], ic_material_options_dialog_png_start[], 
    ic_material_options_dialog_dark_png_start[], ic_material_properties_dialog_png_start[], ic_material_properties_dialog_dark_png_start[], 
    ic_material_dialog_png_start[], ic_material_dialog_dark_png_start[], battery_20_png_start[], battery_20_charging_png_start[], battery_30_png_start[], 
    battery_30_charging_png_start[], battery_50_png_start[], battery_50_charging_png_start[], battery_60_png_start[], battery_60_charging_png_start[], 
    battery_80_png_start[], battery_80_charging_png_start[], battery_full_png_start[], battery_full_charging_png_start[], ic_material_light_usb_png_start[], 
    default_artwork_png_start[], default_artwork_blur_png_start[], btn_playback_play_png_start[], btn_playback_pause_png_start[], 
    btn_playback_rewind_png_start[], btn_playback_forward_png_start[], btn_playback_repeat_png_start[], btn_playback_shuffle_png_start[], 
    btn_playback_repeat_overlay_png_start[], btn_playback_shuffle_overlay_png_start[], bg_header_png_start[], ic_material_light_sdcard_png_start[], 
    ic_material_light_secure_png_start[], ic_material_light_sdcard_dark_png_start[], ic_material_light_secure_dark_png_start[], ic_play_btn_png_start[], 
    ftp_icon_png_start[], sort_icon_png_start[], dark_theme_icon_png_start[], dev_options_icon_png_start[], about_icon_png_start[], 
    ftp_icon_dark_png_start[], sort_icon_dark_png_start[], dark_theme_icon_dark_png_start[], dev_options_icon_dark_png_start[], about_icon_dark_png_start[];

extern unsigned int ic_fso_type_app_png_size, ic_fso_type_compress_png_size, ic_fso_type_audio_png_size, ic_fso_folder_png_size, 
    ic_fso_folder_dark_png_size, ic_fso_default_png_size, ic_fso_type_image_png_size, ic_fso_type_text_png_size, 
    btn_material_light_check_on_normal_png_size, btn_material_light_check_on_normal_dark_png_size, btn_material_light_check_off_normal_png_size, 
    btn_material_light_check_off_normal_dark_png_size, btn_material_light_toggle_on_normal_png_size, btn_material_light_toggle_on_normal_dark_png_size, 
    btn_material_light_toggle_off_normal_png_size, btn_material_light_radio_off_normal_png_size, btn_material_light_radio_on_normal_png_size, 
    btn_material_light_radio_off_normal_dark_png_size, btn_material_light_radio_on_normal_dark_png_size, ic_material_light_navigation_drawer_png_size, 
    ic_arrow_back_normal_png_size, ic_material_options_dialog_png_size, ic_material_options_dialog_dark_png_size, ic_material_properties_dialog_png_size, 
    ic_material_properties_dialog_dark_png_size, ic_material_dialog_png_size, ic_material_dialog_dark_png_size, battery_20_png_size, 
    battery_20_charging_png_size, battery_30_png_size, battery_30_charging_png_size, battery_50_png_size, battery_50_charging_png_size, 
    battery_60_png_size, battery_60_charging_png_size, battery_80_png_size, battery_80_charging_png_size, battery_full_png_size, 
    battery_full_charging_png_size, ic_material_light_usb_png_size, 
    default_artwork_png_size, default_artwork_blur_png_size, btn_playback_play_png_size, btn_playback_pause_png_size, btn_playback_rewind_png_size, 
    btn_playback_forward_png_size, btn_playback_repeat_png_size, btn_playback_shuffle_png_size, btn_playback_repeat_overlay_png_size, 
    btn_playback_shuffle_overlay_png_size, bg_header_png_size, ic_material_light_sdcard_png_size, ic_material_light_secure_png_size, 
    ic_material_light_sdcard_dark_png_size, ic_material_light_secure_dark_png_size, ic_play_btn_png_size, 
    ftp_icon_png_size, sort_icon_png_size, dark_theme_icon_png_size, dev_options_icon_png_size, about_icon_png_size, 
    ftp_icon_dark_png_size, sort_icon_dark_png_size, dark_theme_icon_dark_png_size, dev_options_icon_dark_png_size, about_icon_dark_png_size;

g2dTexture *file_icons[NUM_FILE_ICONS], *icon_dir[NUM_THEMES], *icon_check[NUM_THEMES], *icon_uncheck[NUM_THEMES], \
    *icon_toggle_on[NUM_THEMES], *icon_toggle_off, *icon_radio_off[NUM_THEMES], *icon_radio_on[NUM_THEMES], *icon_nav_drawer, \
    *icon_back, *options_dialog[NUM_THEMES], *properties_dialog[NUM_THEMES], *dialog[NUM_THEMES], \
    *battery_charging[NUM_BATT_ICONS], *battery[NUM_BATT_ICONS], *usb_icon, \
    *default_artwork, *default_artwork_blur, *btn_play, *btn_pause, *btn_rewind, *btn_forward, \
    *btn_repeat, *btn_shuffle, *btn_repeat_overlay, *btn_shuffle_overlay, \
    *bg_header, *icon_sd[NUM_THEMES], *icon_secure[NUM_THEMES], *ic_play_btn, *ftp_icon[NUM_THEMES], *sort_icon[NUM_THEMES], \
    *dark_theme_icon[NUM_THEMES], *dev_options_icon[NUM_THEMES], *about_icon[NUM_THEMES];

namespace Textures {
    void Load(void) {
        file_icons[0] = g2dTexLoadMemory(ic_fso_default_png_start, ic_fso_default_png_size, G2D_SWIZZLE);
        file_icons[1] = g2dTexLoadMemory(ic_fso_type_app_png_start, ic_fso_type_app_png_size, G2D_SWIZZLE);
        file_icons[2] = g2dTexLoadMemory(ic_fso_type_compress_png_start, ic_fso_type_compress_png_size, G2D_SWIZZLE);
        file_icons[3] = g2dTexLoadMemory(ic_fso_type_audio_png_start, ic_fso_type_audio_png_size, G2D_SWIZZLE);
        file_icons[4] = g2dTexLoadMemory(ic_fso_type_image_png_start, ic_fso_type_image_png_size, G2D_SWIZZLE);
        file_icons[5] = g2dTexLoadMemory(ic_fso_type_text_png_start, ic_fso_type_text_png_size, G2D_SWIZZLE);

        icon_dir[0] = g2dTexLoadMemory(ic_fso_folder_png_start, ic_fso_folder_png_size, G2D_SWIZZLE);
        icon_dir[1] = g2dTexLoadMemory(ic_fso_folder_dark_png_start, ic_fso_folder_dark_png_size, G2D_SWIZZLE);
        icon_check[0] = g2dTexLoadMemory(btn_material_light_check_on_normal_png_start, btn_material_light_check_on_normal_png_size, G2D_SWIZZLE);
        icon_check[1] = g2dTexLoadMemory(btn_material_light_check_on_normal_dark_png_start, btn_material_light_check_on_normal_dark_png_size, G2D_SWIZZLE);
        icon_uncheck[0] = g2dTexLoadMemory(btn_material_light_check_off_normal_png_start, btn_material_light_check_off_normal_png_size, G2D_SWIZZLE);
        icon_uncheck[1] = g2dTexLoadMemory(btn_material_light_check_off_normal_dark_png_start, btn_material_light_check_off_normal_dark_png_size, G2D_SWIZZLE);
        icon_toggle_on[0] = g2dTexLoadMemory(btn_material_light_toggle_on_normal_png_start, btn_material_light_toggle_on_normal_png_size, G2D_SWIZZLE);
        icon_toggle_on[1] = g2dTexLoadMemory(btn_material_light_toggle_on_normal_dark_png_start, btn_material_light_toggle_on_normal_dark_png_size, G2D_SWIZZLE);
        icon_toggle_off = g2dTexLoadMemory(btn_material_light_toggle_off_normal_png_start, btn_material_light_toggle_off_normal_png_size, G2D_SWIZZLE);
        icon_radio_off[0] = g2dTexLoadMemory(btn_material_light_radio_off_normal_png_start, btn_material_light_radio_off_normal_png_size, G2D_SWIZZLE);
        icon_radio_off[1] = g2dTexLoadMemory(btn_material_light_radio_off_normal_dark_png_start, btn_material_light_radio_off_normal_dark_png_size, G2D_SWIZZLE);
        icon_radio_on[0] = g2dTexLoadMemory(btn_material_light_radio_on_normal_png_start, btn_material_light_radio_on_normal_png_size, G2D_SWIZZLE);
        icon_radio_on[1] = g2dTexLoadMemory(btn_material_light_radio_on_normal_dark_png_start, btn_material_light_radio_on_normal_dark_png_size, G2D_SWIZZLE);
        icon_nav_drawer = g2dTexLoadMemory(ic_material_light_navigation_drawer_png_start, ic_material_light_navigation_drawer_png_size, G2D_SWIZZLE);
        icon_back = g2dTexLoadMemory(ic_arrow_back_normal_png_start, ic_arrow_back_normal_png_size, G2D_SWIZZLE);
        options_dialog[0] = g2dTexLoadMemory(ic_material_options_dialog_png_start, ic_material_options_dialog_png_size, G2D_SWIZZLE);
        options_dialog[1] = g2dTexLoadMemory(ic_material_options_dialog_dark_png_start, ic_material_options_dialog_dark_png_size, G2D_SWIZZLE);
        properties_dialog[0] = g2dTexLoadMemory(ic_material_properties_dialog_png_start, ic_material_properties_dialog_png_size, G2D_SWIZZLE);
        properties_dialog[1] = g2dTexLoadMemory(ic_material_properties_dialog_dark_png_start, ic_material_properties_dialog_dark_png_size, G2D_SWIZZLE);
        dialog[0] = g2dTexLoadMemory(ic_material_dialog_png_start, ic_material_dialog_png_size, G2D_SWIZZLE);
        dialog[1] = g2dTexLoadMemory(ic_material_dialog_dark_png_start, ic_material_dialog_dark_png_size, G2D_SWIZZLE);

        battery_charging[0] = g2dTexLoadMemory(battery_20_charging_png_start, battery_20_charging_png_size, G2D_SWIZZLE);
        battery_charging[1] = g2dTexLoadMemory(battery_30_charging_png_start, battery_30_charging_png_size, G2D_SWIZZLE);
        battery_charging[2] = g2dTexLoadMemory(battery_50_charging_png_start, battery_50_charging_png_size, G2D_SWIZZLE);
        battery_charging[3] = g2dTexLoadMemory(battery_60_charging_png_start, battery_60_charging_png_size, G2D_SWIZZLE);
        battery_charging[4] = g2dTexLoadMemory(battery_80_charging_png_start, battery_80_charging_png_size, G2D_SWIZZLE);
        battery_charging[5] = g2dTexLoadMemory(battery_full_charging_png_start, battery_full_charging_png_size, G2D_SWIZZLE);
        
        battery[0] = g2dTexLoadMemory(battery_20_png_start, battery_30_png_size, G2D_SWIZZLE);
        battery[1] = g2dTexLoadMemory(battery_30_png_start, battery_30_png_size, G2D_SWIZZLE);
        battery[2] = g2dTexLoadMemory(battery_50_png_start, battery_50_png_size, G2D_SWIZZLE);
        battery[3] = g2dTexLoadMemory(battery_60_png_start, battery_60_png_size, G2D_SWIZZLE);
        battery[4] = g2dTexLoadMemory(battery_80_png_start, battery_80_png_size, G2D_SWIZZLE);
        battery[5] = g2dTexLoadMemory(battery_full_png_start, battery_full_png_size, G2D_SWIZZLE);
        
        usb_icon = g2dTexLoadMemory(ic_material_light_usb_png_start, ic_material_light_usb_png_size, G2D_SWIZZLE);
        
        default_artwork = g2dTexLoadMemory(default_artwork_png_start, default_artwork_png_size, G2D_SWIZZLE);
        default_artwork_blur = g2dTexLoadMemory(default_artwork_blur_png_start, default_artwork_blur_png_size, G2D_SWIZZLE);
        btn_play = g2dTexLoadMemory(btn_playback_play_png_start, btn_playback_play_png_size, G2D_SWIZZLE);
        btn_pause = g2dTexLoadMemory(btn_playback_pause_png_start, btn_playback_pause_png_size, G2D_SWIZZLE);
        btn_rewind = g2dTexLoadMemory(btn_playback_rewind_png_start, btn_playback_rewind_png_size, G2D_SWIZZLE);
        btn_forward = g2dTexLoadMemory(btn_playback_forward_png_start, btn_playback_forward_png_size, G2D_SWIZZLE);
        btn_repeat = g2dTexLoadMemory(btn_playback_repeat_png_start, btn_playback_repeat_png_size, G2D_SWIZZLE);
        btn_shuffle = g2dTexLoadMemory(btn_playback_shuffle_png_start, btn_playback_shuffle_png_size, G2D_SWIZZLE);
        btn_repeat_overlay = g2dTexLoadMemory(btn_playback_repeat_overlay_png_start, btn_playback_repeat_overlay_png_size, G2D_SWIZZLE);
        btn_shuffle_overlay = g2dTexLoadMemory(btn_playback_shuffle_overlay_png_start, btn_playback_shuffle_overlay_png_size, G2D_SWIZZLE);
        
        bg_header = g2dTexLoadMemory(bg_header_png_start, bg_header_png_size, G2D_SWIZZLE);
        icon_sd[0] = g2dTexLoadMemory(ic_material_light_sdcard_png_start, ic_material_light_sdcard_png_size, G2D_SWIZZLE);
        icon_sd[1] = g2dTexLoadMemory(ic_material_light_sdcard_dark_png_start, ic_material_light_sdcard_dark_png_size, G2D_SWIZZLE);
        icon_secure[0] = g2dTexLoadMemory(ic_material_light_secure_png_start, ic_material_light_secure_png_size, G2D_SWIZZLE);
        icon_secure[1] = g2dTexLoadMemory(ic_material_light_secure_dark_png_start, ic_material_light_secure_dark_png_size, G2D_SWIZZLE);
        ic_play_btn = g2dTexLoadMemory(ic_play_btn_png_start, ic_play_btn_png_size, G2D_SWIZZLE);

        ftp_icon[0] = g2dTexLoadMemory(ftp_icon_png_start, ftp_icon_png_size, G2D_SWIZZLE);
        ftp_icon[1] = g2dTexLoadMemory(ftp_icon_dark_png_start, ftp_icon_dark_png_size, G2D_SWIZZLE);
        sort_icon[0] = g2dTexLoadMemory(sort_icon_png_start, sort_icon_png_size, G2D_SWIZZLE);
        sort_icon[1] = g2dTexLoadMemory(sort_icon_dark_png_start, sort_icon_dark_png_size, G2D_SWIZZLE);
        dark_theme_icon[0] = g2dTexLoadMemory(dark_theme_icon_png_start, dark_theme_icon_png_size, G2D_SWIZZLE);
        dark_theme_icon[1] = g2dTexLoadMemory(dark_theme_icon_dark_png_start, dark_theme_icon_dark_png_size, G2D_SWIZZLE);
        dev_options_icon[0] = g2dTexLoadMemory(dev_options_icon_png_start, dev_options_icon_png_size, G2D_SWIZZLE);
        dev_options_icon[1] = g2dTexLoadMemory(dev_options_icon_dark_png_start, dev_options_icon_dark_png_size, G2D_SWIZZLE);
        about_icon[0] = g2dTexLoadMemory(about_icon_png_start, about_icon_png_size, G2D_SWIZZLE);
        about_icon[1] = g2dTexLoadMemory(about_icon_dark_png_start, about_icon_dark_png_size, G2D_SWIZZLE);
    }

    void Free(void) {
        g2dTexFree(&ic_play_btn);
        g2dTexFree(&bg_header);
        g2dTexFree(&btn_shuffle_overlay);
        g2dTexFree(&btn_repeat_overlay);
        g2dTexFree(&btn_shuffle);
        g2dTexFree(&btn_repeat);
        g2dTexFree(&btn_forward);
        g2dTexFree(&btn_rewind);
        g2dTexFree(&btn_pause);
        g2dTexFree(&btn_play);
        g2dTexFree(&default_artwork_blur);
        g2dTexFree(&default_artwork);
        g2dTexFree(&usb_icon);
        g2dTexFree(&icon_back);
        g2dTexFree(&icon_nav_drawer);
        g2dTexFree(&icon_toggle_off);

        for (int i = 0; i < NUM_THEMES; i++) {
            g2dTexFree(&about_icon[i]);
            g2dTexFree(&dev_options_icon[i]);
            g2dTexFree(&sort_icon[i]);
            g2dTexFree(&ftp_icon[i]);
            g2dTexFree(&icon_secure[i]);
            g2dTexFree(&icon_sd[i]);
            g2dTexFree(&dialog[i]);
            g2dTexFree(&properties_dialog[i]);
            g2dTexFree(&options_dialog[i]);
            g2dTexFree(&icon_radio_on[i]);
            g2dTexFree(&icon_radio_off[i]);
            g2dTexFree(&icon_toggle_on[i]);
            g2dTexFree(&icon_uncheck[i]);
            g2dTexFree(&icon_check[i]);
            g2dTexFree(&icon_dir[i]);
        }
        
        for (int i = 0; i < NUM_FILE_ICONS; i++)
            g2dTexFree(&file_icons[i]);
            
        for (int i = 0; i < NUM_BATT_ICONS; i++) {
            g2dTexFree(&battery_charging[i]);
            g2dTexFree(&battery[i]);
        }
    }
}
