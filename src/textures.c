#include "textures.h"

extern unsigned char ic_fso_type_app_png_start[], ic_fso_type_compress_png_start[], ic_fso_type_audio_png_start[], ic_fso_type_cdimage_png_start[], 
	ic_fso_folder_png_start[], ic_fso_folder_dark_png_start[], ic_fso_default_png_start[], ic_fso_type_image_png_start[], ic_fso_type_system_png_start[], 
	ic_fso_type_text_png_start[], btn_material_light_check_on_normal_png_start[], btn_material_light_check_on_normal_dark_png_start[], 
	btn_material_light_check_off_normal_png_start[], btn_material_light_check_off_normal_dark_png_start[], btn_material_light_toggle_on_normal_png_start[], 
	btn_material_light_toggle_on_normal_dark_png_start[], btn_material_light_toggle_off_normal_png_start[], btn_material_light_radio_off_normal_png_start[], 
	btn_material_light_radio_on_normal_png_start[], btn_material_light_radio_off_normal_dark_png_start[], btn_material_light_radio_on_normal_dark_png_start[], 
	ic_material_light_navigation_drawer_png_start[], ic_arrow_back_normal_png_start[], ic_material_options_dialog_png_start[], 
	ic_material_options_dialog_dark_png_start[], ic_material_properties_dialog_png_start[], ic_material_properties_dialog_dark_png_start[], 
	ic_material_dialog_png_start[], ic_material_dialog_dark_png_start[], battery_20_png_start[], battery_20_charging_png_start[], battery_30_png_start[], 
	battery_30_charging_png_start[], battery_50_png_start[], battery_50_charging_png_start[], battery_60_png_start[], battery_60_charging_png_start[], 
	battery_80_png_start[], battery_80_charging_png_start[], battery_90_png_start[], battery_90_charging_png_start[], battery_full_png_start[], 
	battery_full_charging_png_start[], battery_low_png_start[], battery_unknown_png_start[], stat_sys_wifi_signal_off_png_start[], 
	stat_sys_wifi_signal_on_png_start[], ic_material_light_usb_png_start[], default_artwork_png_start[], btn_playback_play_png_start[], btn_playback_pause_png_start[], 
	btn_playback_rewind_png_start[], btn_playback_forward_png_start[], btn_playback_repeat_png_start[], btn_playback_shuffle_png_start[], 
	btn_playback_repeat_overlay_png_start[], btn_playback_shuffle_overlay_png_start[], bg_header_png_start[], ic_material_light_sdcard_png_start[], 
	ic_material_light_secure_png_start[], ic_material_light_sdcard_dark_png_start[], ic_material_light_secure_dark_png_start[];

extern unsigned int ic_fso_type_app_png_size, ic_fso_type_compress_png_size, ic_fso_type_audio_png_size, ic_fso_type_cdimage_png_size, ic_fso_folder_png_size, 
	ic_fso_folder_dark_png_size, ic_fso_default_png_size, ic_fso_type_image_png_size, ic_fso_type_system_png_size, ic_fso_type_text_png_size, 
	btn_material_light_check_on_normal_png_size, btn_material_light_check_on_normal_dark_png_size, btn_material_light_check_off_normal_png_size, 
	btn_material_light_check_off_normal_dark_png_size, btn_material_light_toggle_on_normal_png_size, btn_material_light_toggle_on_normal_dark_png_size, 
	btn_material_light_toggle_off_normal_png_size, btn_material_light_radio_off_normal_png_size, btn_material_light_radio_on_normal_png_size, 
	btn_material_light_radio_off_normal_dark_png_size, btn_material_light_radio_on_normal_dark_png_size, ic_material_light_navigation_drawer_png_size, 
	ic_arrow_back_normal_png_size, ic_material_options_dialog_png_size, ic_material_options_dialog_dark_png_size, ic_material_properties_dialog_png_size, 
	ic_material_properties_dialog_dark_png_size, ic_material_dialog_png_size, ic_material_dialog_dark_png_size, battery_20_png_size, 
	battery_20_charging_png_size, battery_30_png_size, battery_30_charging_png_size, battery_50_png_size, battery_50_charging_png_size, 
	battery_60_png_size, battery_60_charging_png_size, battery_80_png_size, battery_80_charging_png_size, battery_90_png_size, battery_90_charging_png_size, 
	battery_full_png_size, battery_full_charging_png_size, battery_low_png_size, battery_unknown_png_size, stat_sys_wifi_signal_off_png_size, 
	stat_sys_wifi_signal_on_png_size, ic_material_light_usb_png_size, default_artwork_png_size, btn_playback_play_png_size, btn_playback_pause_png_size, 
	btn_playback_rewind_png_size, btn_playback_forward_png_size, btn_playback_repeat_png_size, btn_playback_shuffle_png_size, btn_playback_repeat_overlay_png_size, 
	btn_playback_shuffle_overlay_png_size, bg_header_png_size, ic_material_light_sdcard_png_size, ic_material_light_secure_png_size, 
	ic_material_light_sdcard_dark_png_size, ic_material_light_secure_dark_png_size;

static void oslLoadImageFilePNGMemory(OSL_IMAGE **image, void *data, int size) {
	oslSetTempFileData(data, size, &VF_MEMORY);
	*image = oslLoadImageFilePNG(oslGetTempFileName(), OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
}

void Textures_Load(void) {
	oslLoadImageFilePNGMemory(&icon_app, ic_fso_type_app_png_start, ic_fso_type_app_png_size);
	oslLoadImageFilePNGMemory(&icon_archive, ic_fso_type_compress_png_start, ic_fso_type_compress_png_size);
	oslLoadImageFilePNGMemory(&icon_audio, ic_fso_type_audio_png_start, ic_fso_type_audio_png_size);
	oslLoadImageFilePNGMemory(&icon_cd, ic_fso_type_cdimage_png_start, ic_fso_type_cdimage_png_size);
	oslLoadImageFilePNGMemory(&icon_dir, ic_fso_folder_png_start, ic_fso_folder_png_size);
	oslLoadImageFilePNGMemory(&icon_dir_dark, ic_fso_folder_dark_png_start, ic_fso_folder_dark_png_size);
	oslLoadImageFilePNGMemory(&icon_file, ic_fso_default_png_start, ic_fso_default_png_size);
	oslLoadImageFilePNGMemory(&icon_image, ic_fso_type_image_png_start, ic_fso_type_image_png_size);
	oslLoadImageFilePNGMemory(&icon_prx, ic_fso_type_system_png_start, ic_fso_type_system_png_size);
	oslLoadImageFilePNGMemory(&icon_text, ic_fso_type_text_png_start, ic_fso_type_text_png_size);
	oslLoadImageFilePNGMemory(&icon_check, btn_material_light_check_on_normal_png_start, btn_material_light_check_on_normal_png_size);
	oslLoadImageFilePNGMemory(&icon_check_dark, btn_material_light_check_on_normal_dark_png_start, btn_material_light_check_on_normal_dark_png_size);
	oslLoadImageFilePNGMemory(&icon_uncheck, btn_material_light_check_off_normal_png_start, btn_material_light_check_off_normal_png_size);
	oslLoadImageFilePNGMemory(&icon_uncheck_dark, btn_material_light_check_off_normal_dark_png_start, btn_material_light_check_off_normal_dark_png_size);
	oslLoadImageFilePNGMemory(&icon_toggle_on, btn_material_light_toggle_on_normal_png_start, btn_material_light_toggle_on_normal_png_size);
	oslLoadImageFilePNGMemory(&icon_toggle_dark_on, btn_material_light_toggle_on_normal_dark_png_start, btn_material_light_toggle_on_normal_dark_png_size);
	oslLoadImageFilePNGMemory(&icon_toggle_off, btn_material_light_toggle_off_normal_png_start, btn_material_light_toggle_off_normal_png_size);
	oslLoadImageFilePNGMemory(&icon_radio_off, btn_material_light_radio_off_normal_png_start, btn_material_light_radio_off_normal_png_size);
	oslLoadImageFilePNGMemory(&icon_radio_on, btn_material_light_radio_on_normal_png_start, btn_material_light_radio_on_normal_png_size);
	oslLoadImageFilePNGMemory(&icon_radio_dark_off, btn_material_light_radio_off_normal_dark_png_start, btn_material_light_radio_off_normal_dark_png_size);
	oslLoadImageFilePNGMemory(&icon_radio_dark_on, btn_material_light_radio_on_normal_dark_png_start, btn_material_light_radio_on_normal_dark_png_size);
	oslLoadImageFilePNGMemory(&icon_nav_drawer, ic_material_light_navigation_drawer_png_start, ic_material_light_navigation_drawer_png_size);
	oslLoadImageFilePNGMemory(&icon_back, ic_arrow_back_normal_png_start, ic_arrow_back_normal_png_size);
	oslLoadImageFilePNGMemory(&options_dialog, ic_material_options_dialog_png_start, ic_material_options_dialog_png_size);
	oslLoadImageFilePNGMemory(&options_dialog_dark, ic_material_options_dialog_dark_png_start, ic_material_options_dialog_dark_png_size);
	oslLoadImageFilePNGMemory(&properties_dialog, ic_material_properties_dialog_png_start, ic_material_properties_dialog_png_size);
	oslLoadImageFilePNGMemory(&properties_dialog_dark, ic_material_properties_dialog_dark_png_start, ic_material_properties_dialog_dark_png_size);
	oslLoadImageFilePNGMemory(&dialog, ic_material_dialog_png_start, ic_material_dialog_png_size);
	oslLoadImageFilePNGMemory(&dialog_dark, ic_material_dialog_dark_png_start, ic_material_dialog_dark_png_size);
	oslLoadImageFilePNGMemory(&battery_20, battery_20_png_start, battery_20_png_size);
	oslLoadImageFilePNGMemory(&battery_20_charging, battery_20_charging_png_start, battery_20_charging_png_size);
	oslLoadImageFilePNGMemory(&battery_30, battery_30_png_start, battery_30_png_size);
	oslLoadImageFilePNGMemory(&battery_30_charging, battery_30_charging_png_start, battery_30_charging_png_size);
	oslLoadImageFilePNGMemory(&battery_50, battery_50_png_start, battery_50_png_size);
	oslLoadImageFilePNGMemory(&battery_50_charging, battery_50_charging_png_start, battery_50_charging_png_size);
	oslLoadImageFilePNGMemory(&battery_60, battery_60_png_start, battery_60_png_size);
	oslLoadImageFilePNGMemory(&battery_60_charging, battery_60_charging_png_start, battery_60_charging_png_size);
	oslLoadImageFilePNGMemory(&battery_80, battery_80_png_start, battery_80_png_size);
	oslLoadImageFilePNGMemory(&battery_80_charging, battery_80_charging_png_start, battery_80_charging_png_size);
	oslLoadImageFilePNGMemory(&battery_90, battery_90_png_start, battery_90_png_size);
	oslLoadImageFilePNGMemory(&battery_90_charging, battery_90_charging_png_start, battery_90_charging_png_size);
	oslLoadImageFilePNGMemory(&battery_full, battery_full_png_start, battery_full_png_size);
	oslLoadImageFilePNGMemory(&battery_full_charging, battery_full_charging_png_start, battery_full_charging_png_size);
	oslLoadImageFilePNGMemory(&battery_low, battery_low_png_start, battery_low_png_size);
	oslLoadImageFilePNGMemory(&battery_unknown, battery_unknown_png_start, battery_unknown_png_size);
	oslLoadImageFilePNGMemory(&wifi_off, stat_sys_wifi_signal_off_png_start, stat_sys_wifi_signal_off_png_size);
	oslLoadImageFilePNGMemory(&wifi_on, stat_sys_wifi_signal_on_png_start, stat_sys_wifi_signal_on_png_size);
	oslLoadImageFilePNGMemory(&usb_icon, ic_material_light_usb_png_start, ic_material_light_usb_png_size);
	oslLoadImageFilePNGMemory(&default_artwork, default_artwork_png_start, default_artwork_png_size);
	oslLoadImageFilePNGMemory(&btn_play, btn_playback_play_png_start, btn_playback_play_png_size);
	oslLoadImageFilePNGMemory(&btn_pause, btn_playback_pause_png_start, btn_playback_pause_png_size);
	oslLoadImageFilePNGMemory(&btn_rewind, btn_playback_rewind_png_start, btn_playback_rewind_png_size);
	oslLoadImageFilePNGMemory(&btn_forward, btn_playback_forward_png_start, btn_playback_forward_png_size);
	oslLoadImageFilePNGMemory(&btn_repeat, btn_playback_repeat_png_start, btn_playback_repeat_png_size);
	oslLoadImageFilePNGMemory(&btn_shuffle, btn_playback_shuffle_png_start, btn_playback_shuffle_png_size);
	oslLoadImageFilePNGMemory(&btn_repeat_overlay, btn_playback_repeat_overlay_png_start, btn_playback_repeat_overlay_png_size);
	oslLoadImageFilePNGMemory(&btn_shuffle_overlay, btn_playback_shuffle_overlay_png_start, btn_playback_shuffle_overlay_png_size);
	oslLoadImageFilePNGMemory(&bg_header, bg_header_png_start, bg_header_png_size);
	oslLoadImageFilePNGMemory(&icon_sd, ic_material_light_sdcard_png_start, ic_material_light_sdcard_png_size);
	oslLoadImageFilePNGMemory(&icon_secure, ic_material_light_secure_png_start, ic_material_light_secure_png_size);
	oslLoadImageFilePNGMemory(&icon_sd_dark, ic_material_light_sdcard_dark_png_start, ic_material_light_sdcard_dark_png_size);
	oslLoadImageFilePNGMemory(&icon_secure_dark, ic_material_light_secure_dark_png_start, ic_material_light_secure_dark_png_size);
}

void Textures_Free(void) {
	oslDeleteImage(icon_secure_dark);
	oslDeleteImage(icon_sd_dark);
	oslDeleteImage(icon_secure);
	oslDeleteImage(icon_sd);
	oslDeleteImage(bg_header);
	oslDeleteImage(btn_shuffle_overlay);
	oslDeleteImage(btn_repeat_overlay);
	oslDeleteImage(btn_shuffle);
	oslDeleteImage(btn_repeat);
	oslDeleteImage(btn_forward);
	oslDeleteImage(btn_rewind);
	oslDeleteImage(btn_pause);
	oslDeleteImage(btn_play);
	oslDeleteImage(default_artwork);
	oslDeleteImage(usb_icon);
	oslDeleteImage(wifi_on);
	oslDeleteImage(wifi_off);
	oslDeleteImage(battery_unknown);
	oslDeleteImage(battery_low);
	oslDeleteImage(battery_full_charging);
	oslDeleteImage(battery_full);
	oslDeleteImage(battery_90_charging);
	oslDeleteImage(battery_80_charging);
	oslDeleteImage(battery_80);
	oslDeleteImage(battery_60_charging);
	oslDeleteImage(battery_60);
	oslDeleteImage(battery_50_charging);
	oslDeleteImage(battery_50);
	oslDeleteImage(battery_30_charging);
	oslDeleteImage(battery_30);
	oslDeleteImage(battery_20_charging);
	oslDeleteImage(battery_20);
	oslDeleteImage(dialog_dark);
	oslDeleteImage(dialog);
	oslDeleteImage(properties_dialog_dark);
	oslDeleteImage(properties_dialog);
	oslDeleteImage(options_dialog_dark);
	oslDeleteImage(options_dialog);
	oslDeleteImage(icon_back);
	oslDeleteImage(icon_nav_drawer);
	oslDeleteImage(icon_radio_dark_on);
	oslDeleteImage(icon_radio_dark_off);
	oslDeleteImage(icon_radio_on);
	oslDeleteImage(icon_radio_off);
	oslDeleteImage(icon_toggle_off);
	oslDeleteImage(icon_toggle_dark_on);
	oslDeleteImage(icon_toggle_on);
	oslDeleteImage(icon_uncheck_dark);
	oslDeleteImage(icon_uncheck);
	oslDeleteImage(icon_check_dark);
	oslDeleteImage(icon_check);
	oslDeleteImage(icon_text);
	oslDeleteImage(icon_prx);
	oslDeleteImage(icon_image);
	oslDeleteImage(icon_file);
	oslDeleteImage(icon_dir_dark);
	oslDeleteImage(icon_dir);
	oslDeleteImage(icon_cd);
	oslDeleteImage(icon_audio);
	oslDeleteImage(icon_archive);
	oslDeleteImage(icon_app);
}
