TARGET = CMFileManager

SOURCES   := data drivers source source/audio source/gui ../libs/libnsbmp ../libs/libnsgif
CFILES    := $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
SFILES    := $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.S))
CPPFILES  := $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))
FONTFILES := $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.pgf))
GFXFILES  := $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.png))
PRXFILES  := $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.prx))

OBJS := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(SFILES:.S=.o) $(CPPFILES:.cpp=.o) \
        $(FONTFILES:.pgf=.o) $(GFXFILES:.png=.o) $(PRXFILES:.prx=.o)

VERSION_MAJOR :=  4
VERSION_MINOR :=  1
VERSION_MICRO :=  0

INCDIR   = ../libs/ ../libs/include ../libs/libnsbmp ../libs/libnsgif ../libs/include/opus include
CFLAGS   = -Os -Wall -ffast-math -Wno-narrowing -Wno-unused-variable \
           -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DVERSION_MICRO=$(VERSION_MICRO)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -std=gnu++17
ASFLAGS  := $(CFLAGS)

BUILD_PRX = 1
PSP_LARGE_MEMORY = 1

LIBDIR  = ../libs/lib
LDFLAGS =
LIBS    = -lintrafont -lglib2d -lxmp -lmpg123 -lvorbisfile -lvorbis -lopusfile -lopus -lFLAC -logg \
          -larchive -llzma -ljpeg -lpng16 -lstdc++ -lz \
          -lpspkubridge -lpspsystemctrl_user -lpspusbdevice \
          -lpspgu -lpspvram -lpspaudio -lpsppower -lpspreg -lpspusb -lpspusbstor -lpspumd

EXTRA_TARGETS   = EBOOT.PBP
PSP_EBOOT_TITLE = CM File Manager PSP v$(VERSION_MAJOR).$(VERSION_MINOR)$(VERSION_MICRO)
PSP_EBOOT_ICON  = ../ICON0.PNG

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

%.o: %.pgf
	bin2o -i $< $@ $(addsuffix _pgf, $(basename $(notdir $<) ))

%.o: %.png
	bin2o -i $< $@ $(addsuffix _png, $(basename $(notdir $<) ))

%.o: %.prx
	bin2o -i $< $@ $(addsuffix _prx, $(basename $(notdir $<) ))