TARGET = CMFileManager

OBJS   = src/config.o src/dirbrowse.o src/fs.o src/log.o src/main.o src/osl_helper.o src/progress_bar.o src/screenshot.o \
         src/status_bar.o src/textures.o src/utils.o \
         src/menus/menu_fileoptions.o src/menus/menu_gallery.o src/menus/menu_main.o src/menus/menu_music.o src/menus/menu_settings.o \
         src/archive/archive.o src/archive/ioapi.o src/archive/unzip.o 

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

VERSION_MAJOR :=  1
VERSION_MINOR :=  0
VERSION_MICRO :=  1

INCDIR   = common include include/archive include/menus
CFLAGS   = -G0 -Wall -O3 -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DVERSION_MICRO=$(VERSION_MICRO)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS  := $(CFLAGS)
OBJS     += $(DATA_OBJS)

LIBDIR  = libs
LDFLAGS =
LIBS    = -losl -lpng -lz -lm -lpspvram \
          -lpsphprm -lpspsdk -lpspctrl -lpsprtc -lpsppower -lpspgu -lpspgum -lpsphttp -lpspssl -lpspwlan \
          -lpspnet_adhocmatching -lpspnet_adhoc -lpspnet_adhocctl -ljpeg \
          -lpspusb -lpspusbstor \
          -lpspmp3 -lmad -lpspaudiolib -lpspaudio -lpspaudiocodec \
          -lpspsystemctrl_user -lpspkubridge

EXTRA_TARGETS    = EBOOT.PBP
PSP_EBOOT_TITLE  = CM File Manager PSP
PSP_EBOOT_ICON   = ICON0.PNG

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

%.o: %.png
	bin2o -i $< $@ $(addsuffix _png, $(basename $(notdir $<) ))

%.o: %.pgf
	bin2o -i $< $@ $(addsuffix _pgf, $(basename $(notdir $<) ))
