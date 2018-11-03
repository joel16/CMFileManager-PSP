TARGET = CMFileManager
OBJS   = src/config.o src/dirbrowse.o src/fs.o src/log.o src/main.o src/osl_helper.o src/progress_bar.o src/screenshot.o \
         src/status_bar.o src/textures.o src/utils.o \
         src/menus/menu_fileoptions.o src/menus/menu_gallery.o src/menus/menu_main.o src/menus/menu_music.o src/menus/menu_settings.o \
         src/archive/archive.o src/archive/ioapi.o src/archive/unzip.o 

#To build for custom firmware:
BUILD_PRX = 1

VERSION_MAJOR :=  1
VERSION_MINOR :=  0
VERSION_MICRO :=  0

INCDIR   = common include include/archive include/menus
CFLAGS   = -O2 -G0 -Wall -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DVERSION_MICRO=$(VERSION_MICRO)
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11
ASFLAGS  := -g $(ARCH)

LIBDIR  = libs
LIBS    = -losl -lpng -lz \
          -lpsphprm -lpspsdk -lpspctrl -lpsprtc -lpsppower -lpspgu -lpspgum -lpsphttp -lpspssl -lpspwlan \
          -lpspnet_adhocmatching -lpspnet_adhoc -lpspnet_adhocctl -lm -ljpeg \
          -lpspusb -lpspusbstor \
          -lpspmp3 -lmad -lpspaudiolib -lpspaudio -lpspaudiocodec \
          -lpspsystemctrl_user -lpspkubridge

EXTRA_TARGETS   = EBOOT.PBP
PSP_EBOOT_TITLE = CM File Manager PSP
PSP_EBOOT_ICON = ICON0.PNG
PSP_LARGE_MEMORY = 1
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
