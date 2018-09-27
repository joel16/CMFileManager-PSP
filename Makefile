TARGET = CMFileManager
OBJS   = src/config.o src/dirbrowse.o src/fs.o src/main.o src/status_bar.o src/textures.o src/utils.o \
         src/menus/menu_fileoptions.o src/menus/menu_main.o src/menus/menu_settings.o

#To build for custom firmware:
BUILD_PRX = 1

VERSION_MAJOR :=  1
VERSION_MINOR :=  0
VERSION_MICRO :=  0

CFLAGS   = -O2 -G0 -Wall -Werror -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DVERSION_MICRO=$(VERSION_MICRO)
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11
ASFLAGS	:=	-g $(ARCH)
LIBDIR   =

STDLIBS = -losl -lpng -lz \
          -lpsphprm -lpspsdk -lpspctrl -lpsppower -lpspgu -lpspgum  -lpspaudiolib -lpspaudio -lpsphttp -lpspssl -lpspwlan \
          -lpspnet_adhocmatching -lpspnet_adhoc -lpspnet_adhocctl -lm -ljpeg
LIBS    = $(STDLIBS)

EXTRA_TARGETS   = EBOOT.PBP
PSP_EBOOT_TITLE = CMFileManager
#PSP_EBOOT_ICON = ICON0.PNG
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak