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
    ic_material_light_secure_png_start[], ic_material_light_sdcard_dark_png_start[], ic_material_light_secure_dark_png_start[], ic_play_btn_png_start[];

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
    ic_material_light_sdcard_dark_png_size, ic_material_light_secure_dark_png_size, ic_play_btn_png_size;

g2dTexture *file_icons[NUM_ICONS], *icon_dir, *icon_dir_dark, *icon_check, *icon_check_dark, *icon_uncheck, \
    *icon_uncheck_dark, *icon_toggle_on, *icon_toggle_dark_on, *icon_toggle_off, *icon_radio_off, \
    *icon_radio_on, *icon_radio_dark_off, *icon_radio_dark_on, *icon_nav_drawer, *icon_back, \
    *options_dialog, *options_dialog_dark, *properties_dialog, *properties_dialog_dark, *dialog, *dialog_dark, \
    *battery_charging[NUM_BATT_ICONS], *battery[NUM_BATT_ICONS], *usb_icon, \
    *default_artwork, *default_artwork_blur, *btn_play, *btn_pause, *btn_rewind, *btn_forward, \
    *btn_repeat, *btn_shuffle, *btn_repeat_overlay, *btn_shuffle_overlay, \
    *bg_header, *icon_sd, *icon_secure, *icon_sd_dark, *icon_secure_dark, *ic_play_btn;

namespace Textures {
    void Load(void) {
        file_icons[0] = g2dTexLoadMemory(ic_fso_default_png_start, ic_fso_default_png_size, G2D_SWIZZLE);
        file_icons[1] = g2dTexLoadMemory(ic_fso_type_app_png_start, ic_fso_type_app_png_size, G2D_SWIZZLE);
        file_icons[2] = g2dTexLoadMemory(ic_fso_type_compress_png_start, ic_fso_type_compress_png_size, G2D_SWIZZLE);
        file_icons[3] = g2dTexLoadMemory(ic_fso_type_audio_png_start, ic_fso_type_audio_png_size, G2D_SWIZZLE);
        file_icons[4] = g2dTexLoadMemory(ic_fso_type_image_png_start, ic_fso_type_image_png_size, G2D_SWIZZLE);
        file_icons[5] = g2dTexLoadMemory(ic_fso_type_text_png_start, ic_fso_type_text_png_size, G2D_SWIZZLE);
        
        icon_dir = g2dTexLoadMemory(ic_fso_folder_png_start, ic_fso_folder_png_size, G2D_SWIZZLE);
        icon_dir_dark = g2dTexLoadMemory(ic_fso_folder_dark_png_start, ic_fso_folder_dark_png_size, G2D_SWIZZLE);
        icon_check = g2dTexLoadMemory(btn_material_light_check_on_normal_png_start, btn_material_light_check_on_normal_png_size, G2D_SWIZZLE);
        icon_check_dark = g2dTexLoadMemory(btn_material_light_check_on_normal_dark_png_start, btn_material_light_check_on_normal_dark_png_size, G2D_SWIZZLE);
        icon_uncheck = g2dTexLoadMemory(btn_material_light_check_off_normal_png_start, btn_material_light_check_off_normal_png_size, G2D_SWIZZLE);
        icon_uncheck_dark = g2dTexLoadMemory(btn_material_light_check_off_normal_dark_png_start, btn_material_light_check_off_normal_dark_png_size, G2D_SWIZZLE);
        icon_toggle_on = g2dTexLoadMemory(btn_material_light_toggle_on_normal_png_start, btn_material_light_toggle_on_normal_png_size, G2D_SWIZZLE);
        icon_toggle_dark_on = g2dTexLoadMemory(btn_material_light_toggle_on_normal_dark_png_start, btn_material_light_toggle_on_normal_dark_png_size, G2D_SWIZZLE);
        icon_toggle_off = g2dTexLoadMemory(btn_material_light_toggle_off_normal_png_start, btn_material_light_toggle_off_normal_png_size, G2D_SWIZZLE);
        icon_radio_off = g2dTexLoadMemory(btn_material_light_radio_off_normal_png_start, btn_material_light_radio_off_normal_png_size, G2D_SWIZZLE);
        icon_radio_on = g2dTexLoadMemory(btn_material_light_radio_on_normal_png_start, btn_material_light_radio_on_normal_png_size, G2D_SWIZZLE);
        icon_radio_dark_off = g2dTexLoadMemory(btn_material_light_radio_off_normal_dark_png_start, btn_material_light_radio_off_normal_dark_png_size, G2D_SWIZZLE);
        icon_radio_dark_on = g2dTexLoadMemory(btn_material_light_radio_on_normal_dark_png_start, btn_material_light_radio_on_normal_dark_png_size, G2D_SWIZZLE);
        icon_nav_drawer = g2dTexLoadMemory(ic_material_light_navigation_drawer_png_start, ic_material_light_navigation_drawer_png_size, G2D_SWIZZLE);
        icon_back = g2dTexLoadMemory(ic_arrow_back_normal_png_start, ic_arrow_back_normal_png_size, G2D_SWIZZLE);
        options_dialog = g2dTexLoadMemory(ic_material_options_dialog_png_start, ic_material_options_dialog_png_size, G2D_SWIZZLE);
        options_dialog_dark = g2dTexLoadMemory(ic_material_options_dialog_dark_png_start, ic_material_options_dialog_dark_png_size, G2D_SWIZZLE);
        properties_dialog = g2dTexLoadMemory(ic_material_properties_dialog_png_start, ic_material_properties_dialog_png_size, G2D_SWIZZLE);
        properties_dialog_dark = g2dTexLoadMemory(ic_material_properties_dialog_dark_png_start, ic_material_properties_dialog_dark_png_size, G2D_SWIZZLE);
        dialog = g2dTexLoadMemory(ic_material_dialog_png_start, ic_material_dialog_png_size, G2D_SWIZZLE);
        dialog_dark = g2dTexLoadMemory(ic_material_dialog_dark_png_start, ic_material_dialog_dark_png_size, G2D_SWIZZLE);
        
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
        icon_sd = g2dTexLoadMemory(ic_material_light_sdcard_png_start, ic_material_light_sdcard_png_size, G2D_SWIZZLE);
        icon_secure = g2dTexLoadMemory(ic_material_light_secure_png_start, ic_material_light_secure_png_size, G2D_SWIZZLE);
        icon_sd_dark = g2dTexLoadMemory(ic_material_light_sdcard_dark_png_start, ic_material_light_sdcard_dark_png_size, G2D_SWIZZLE);
        icon_secure_dark = g2dTexLoadMemory(ic_material_light_secure_dark_png_start, ic_material_light_secure_dark_png_size, G2D_SWIZZLE);
        ic_play_btn = g2dTexLoadMemory(ic_play_btn_png_start, ic_play_btn_png_size, G2D_SWIZZLE);
    }

    void Free(void) {
        g2dTexFree(&ic_play_btn);
        g2dTexFree(&icon_secure_dark);
        g2dTexFree(&icon_sd_dark);
        g2dTexFree(&icon_secure);
        g2dTexFree(&icon_sd);
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
        g2dTexFree(&dialog_dark);
        g2dTexFree(&dialog);
        g2dTexFree(&properties_dialog_dark);
        g2dTexFree(&properties_dialog);
        g2dTexFree(&options_dialog_dark);
        g2dTexFree(&options_dialog);
        g2dTexFree(&icon_back);
        g2dTexFree(&icon_nav_drawer);
        g2dTexFree(&icon_radio_dark_on);
        g2dTexFree(&icon_radio_dark_off);
        g2dTexFree(&icon_radio_on);
        g2dTexFree(&icon_radio_off);
        g2dTexFree(&icon_toggle_off);
        g2dTexFree(&icon_toggle_dark_on);
        g2dTexFree(&icon_toggle_on);
        g2dTexFree(&icon_uncheck_dark);
        g2dTexFree(&icon_uncheck);
        g2dTexFree(&icon_check_dark);
        g2dTexFree(&icon_check);
        g2dTexFree(&icon_dir_dark);
        g2dTexFree(&icon_dir);
        
        for (int i = 0; i < NUM_ICONS; i++)
            g2dTexFree(&file_icons[i]);
            
        for (int i = 0; i < NUM_BATT_ICONS; i++) {
            g2dTexFree(&battery_charging[i]);
            g2dTexFree(&battery[i]);
        }
    }
}
