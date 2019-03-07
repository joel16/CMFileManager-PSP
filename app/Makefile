TARGET = CMFileManager

SOURCES   := data glib2d source source/archive source/menus
CFILES    := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES  := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
GFXFILES  := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.png))
FONTFILES := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.pgf))

OBJS := $(addsuffix .o,$(BINFILES)) \
        $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) $(GFXFILES:.png=.o) $(FONTFILES:.pgf=.o)

PSP_LARGE_MEMORY = 1

VERSION_MAJOR :=  2
VERSION_MINOR :=  0
VERSION_MICRO :=  0

INCDIR   = ../common include include/archive include/menus glib2d
CFLAGS   = -G0 -Wall -Os -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DVERSION_MICRO=$(VERSION_MICRO)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS  := $(CFLAGS)

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
