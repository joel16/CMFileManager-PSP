TARGET = CMFileManager

SOURCES   := data glib2d source source/archive source/audio source/menus
CFILES    := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
SFILES    := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.S))
CPPFILES  := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
GFXFILES  := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.png))
FONTFILES := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.pgf))

OBJS := $(addsuffix .o,$(BINFILES)) \
        $(CFILES:.c=.o) $(SFILES:.S=.o) $(CPPFILES:.cpp=.o) $(GFXFILES:.png=.o) $(FONTFILES:.pgf=.o)

PSP_LARGE_MEMORY = 1

VERSION_MAJOR :=  2
VERSION_MINOR :=  2
VERSION_MICRO :=  0

INCDIR   = ../common include include/archive include/audio include/menus glib2d
CFLAGS   = -O3 -G0 -Wall -Werror -ffast-math -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DVERSION_MICRO=$(VERSION_MICRO)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS  := $(CFLAGS)

LIBDIR  = ../libs
LDFLAGS =
LIBS    = -lz -lpspgum -lpspgu -lpsprtc -lm -lpspvram -lpsphttp -lpspssl -lpspwlan -lxmp-lite -lmpg123 -lpspaudiolib -lpspaudio \
          -lpspsdk -lpspctrl -lpsppower -lpspnet_adhocmatching -lpspnet_adhoc -lpspnet_adhocctl \
          -lpspreg -lpspusb -lpspusbstor -lpspusbdevice -lpspumd -lpspkubridge -lpspsystemctrl_user -lm

EXTRA_TARGETS    = EBOOT.PBP
PSP_EBOOT_TITLE  = CM File Manager PSP
PSP_EBOOT_ICON   = ../ICON0.PNG

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

%.o: %.png
	bin2o -i $< $@ $(addsuffix _png, $(basename $(notdir $<) ))

%.o: %.pgf
	bin2o -i $< $@ $(addsuffix _pgf, $(basename $(notdir $<) ))
