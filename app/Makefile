TARGET = CMFileManager

OBJS   = glib2d/glib2d.o glib2d/intraFont.o glib2d/libccc.o \
         source/common.o source/config.o source/dirbrowse.o source/fs.o source/log.o source/main.o source/glib2d_helper.o source/progress_bar.o \
         source/screenshot.o source/status_bar.o source/textures.o source/utils.o source/menus/menu_error.o source/menus/menu_fileoptions.o \
         source/menus/menu_gallery.o source/menus/menu_main.o source/menus/menu_settings.o source/archive/archive.o source/archive/ioapi.o source/archive/unzip.o 

DATA_OBJS = data/battery_20.o data/battery_30.o data/battery_50.o data/battery_60.o data/battery_80.o data/battery_90.o data/battery_full.o \
            data/battery_low.o data/battery_unknown.o data/battery_20_charging.o data/battery_30_charging.o data/battery_50_charging.o \
            data/battery_60_charging.o data/battery_80_charging.o data/battery_90_charging.o data/battery_full_charging.o data/bg_header.o \
            data/btn_material_light_check_off_normal.o data/btn_material_light_check_off_normal_dark.o data/btn_material_light_check_on_normal.o \
            data/btn_material_light_check_on_normal_dark.o data/btn_material_light_radio_off_normal.o data/btn_material_light_radio_off_normal_dark.o \
            data/btn_material_light_radio_on_normal.o data/btn_material_light_radio_on_normal_dark.o data/btn_material_light_toggle_off_normal.o \
            data/btn_material_light_toggle_on_normal.o data/btn_material_light_toggle_on_normal_dark.o data/btn_playback_forward.o \
            data/btn_playback_pause.o data/btn_playback_play.o data/btn_playback_repeat.o data/btn_playback_repeat_overlay.o data/btn_playback_rewind.o \
            data/btn_playback_shuffle.o data/btn_playback_shuffle_overlay.o data/default_artwork.o data/ic_arrow_back_normal.o data/ic_fso_default.o \
            data/ic_fso_folder.o data/ic_fso_folder_dark.o data/ic_fso_type_app.o data/ic_fso_type_audio.o data/ic_fso_type_cdimage.o \
            data/ic_fso_type_compress.o data/ic_fso_type_image.o data/ic_fso_type_system.o data/ic_fso_type_text.o data/ic_material_dialog.o \
            data/ic_material_dialog_dark.o data/ic_material_light_navigation_drawer.o data/ic_material_light_sdcard.o data/ic_material_light_sdcard_dark.o \
            data/ic_material_light_secure.o data/ic_material_light_secure_dark.o data/ic_material_light_usb.o data/ic_material_options_dialog.o \
            data/ic_material_options_dialog_dark.o data/ic_material_properties_dialog.o data/ic_material_properties_dialog_dark.o \
            data/stat_sys_wifi_signal_off.o data/stat_sys_wifi_signal_on.o data/Roboto.o

PSP_LARGE_MEMORY = 1

VERSION_MAJOR :=  2
VERSION_MINOR :=  0
VERSION_MICRO :=  0

INCDIR   = ../common include include/archive include/menus glib2d
CFLAGS   = -G0 -Wall -Os -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DVERSION_MICRO=$(VERSION_MICRO)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS  := $(CFLAGS)
OBJS     += $(DATA_OBJS)

LIBDIR  = ../libs
LDFLAGS =
LIBS    = -lz -lpspgum -lpspgu -lpsprtc -lm -lpspvram -lpsphttp -lpspssl -lpspwlan \
          -lpspsdk -lpspctrl -lpsppower -lpspnet_adhocmatching -lpspnet_adhoc -lpspnet_adhocctl \
          -lpspreg -lpspusb -lpspusbstor -lpspusbdevice -lpspumd -lpspkubridge -lpspsystemctrl_user

EXTRA_TARGETS    = EBOOT.PBP
PSP_EBOOT_TITLE  = CM File Manager PSP
PSP_EBOOT_ICON   = ../ICON0.PNG

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

%.o: %.png
	bin2o -i $< $@ $(addsuffix _png, $(basename $(notdir $<) ))

%.o: %.pgf
	bin2o -i $< $@ $(addsuffix _pgf, $(basename $(notdir $<) ))
