TARGET = CMFileManager

SOURCES   := data drivers source source/audio source/gui
CFILES    := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
SFILES    := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.S))
CPPFILES  := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
GFXFILES  := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.png))
FONTFILES := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.pgf))

OBJS := $(addsuffix .o,$(BINFILES)) \
        $(CFILES:.c=.o) $(SFILES:.S=.o) $(CPPFILES:.cpp=.o) $(GFXFILES:.png=.o) $(FONTFILES:.pgf=.o)

VERSION_MAJOR :=  4
VERSION_MINOR :=  0
VERSION_MICRO :=  0

INCDIR   = ../libs/include ../libs/include/opus include
CFLAGS   = -Os -G0 -Wall -ffast-math -fno-rtti -fno-exceptions -Wno-narrowing -Wno-unused-variable \
           -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DVERSION_MICRO=$(VERSION_MICRO)
CXXFLAGS = $(CFLAGS) -std=gnu++17
ASFLAGS  := $(CFLAGS)

BUILD_PRX = 1
PSP_LARGE_MEMORY = 1

LIBDIR  = ../libs/lib
LDFLAGS =
LIBS    = -lpsputility -lintrafont -lglib2d -lpspgu -lpsprtc -lpspvram \
          -lxmp-lite -lmpg123 -lvorbisfile -lvorbis -lopusfile -lopus -lFLAC -logg -lpspaudio \
          -lpspctrl -lpsppower -lpspreg -lpspusb -lpspusbstor -lpspusbdevice -lpspumd \
          -lpspkubridge -lpspsystemctrl_user -lstdc++ -lm -lz

EXTRA_TARGETS    = EBOOT.PBP
PSP_EBOOT_TITLE  = CM File Manager PSP v$(VERSION_MAJOR).$(VERSION_MINOR)$(VERSION_MICRO)
PSP_EBOOT_ICON   = ../ICON0.PNG

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

%.o: %.png
	bin2o -i $< $@ $(addsuffix _png, $(basename $(notdir $<) ))

%.o: %.pgf
	bin2o -i $< $@ $(addsuffix _pgf, $(basename $(notdir $<) ))
