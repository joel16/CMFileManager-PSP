#include "textures.h"

void Textures_Load(void) {
	icon_file = oslLoadImageFilePNG("data/ic_fso_default.png", OSL_IN_RAM, OSL_PF_8888);
	icon_dir = oslLoadImageFilePNG("data/ic_fso_folder.png", OSL_IN_RAM, OSL_PF_8888);
	icon_dir_dark = oslLoadImageFilePNG("data/ic_fso_folder_dark.png", OSL_IN_RAM, OSL_PF_8888);
	icon_check = oslLoadImageFilePNG("data/btn_material_light_check_on_normal.png", OSL_IN_VRAM, OSL_PF_8888);
	icon_check_dark = oslLoadImageFilePNG("data/btn_material_light_check_on_normal_dark.png", OSL_IN_VRAM, OSL_PF_8888);
	icon_uncheck = oslLoadImageFilePNG("data/btn_material_light_check_off_normal.png", OSL_IN_VRAM, OSL_PF_8888);
	icon_uncheck_dark = oslLoadImageFilePNG("data/btn_material_light_check_off_normal_dark.png", OSL_IN_VRAM, OSL_PF_8888);
	icon_toggle_on = oslLoadImageFilePNG("data/btn_material_light_toggle_on_normal.png", OSL_IN_VRAM, OSL_PF_8888);
	icon_toggle_dark_on = oslLoadImageFilePNG("data/btn_material_light_toggle_on_normal_dark.png", OSL_IN_VRAM, OSL_PF_8888);
	icon_toggle_off = oslLoadImageFilePNG("data/btn_material_light_toggle_off_normal.png", OSL_IN_VRAM, OSL_PF_8888);
	icon_radio_off = oslLoadImageFilePNG("data/btn_material_light_radio_off_normal.png", OSL_IN_VRAM, OSL_PF_8888);
	icon_radio_on = oslLoadImageFilePNG("data/btn_material_light_radio_on_normal.png", OSL_IN_VRAM, OSL_PF_8888);
	icon_radio_dark_off = oslLoadImageFilePNG("data/btn_material_light_radio_off_normal_dark.png", OSL_IN_VRAM, OSL_PF_8888);
	icon_radio_dark_on = oslLoadImageFilePNG("data/btn_material_light_radio_on_normal_dark.png", OSL_IN_VRAM, OSL_PF_8888);
	icon_nav_drawer = oslLoadImageFilePNG("data/ic_material_light_navigation_drawer.png", OSL_IN_VRAM, OSL_PF_8888);
	icon_back = oslLoadImageFilePNG("data/ic_arrow_back_normal.png", OSL_IN_VRAM, OSL_PF_8888);
	options_dialog = oslLoadImageFilePNG("data/ic_material_options_dialog.png", OSL_IN_RAM, OSL_PF_8888);
	options_dialog_dark = oslLoadImageFilePNG("data/ic_material_options_dialog_dark.png", OSL_IN_RAM, OSL_PF_8888);
	properties_dialog = oslLoadImageFilePNG("data/ic_material_properties_dialog.png", OSL_IN_RAM, OSL_PF_8888);
	properties_dialog_dark = oslLoadImageFilePNG("data/ic_material_properties_dialog_dark.png", OSL_IN_RAM, OSL_PF_8888);
	dialog = oslLoadImageFilePNG("data/ic_material_dialog.png", OSL_IN_RAM, OSL_PF_8888);
	dialog_dark = oslLoadImageFilePNG("data/ic_material_dialog_dark.png", OSL_IN_RAM, OSL_PF_8888);
}

void Textures_Free(void) {
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
	oslDeleteImage(icon_dir_dark);
	oslDeleteImage(icon_dir);
	oslDeleteImage(icon_file);
}