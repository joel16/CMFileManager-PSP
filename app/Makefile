TARGET = CMFileManager

SOURCES   := data drivers source source/audio source/menus
CFILES    := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
SFILES    := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.S))
CPPFILES  := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
GFXFILES  := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.png))
FONTFILES := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.pgf))

OBJS := $(addsuffix .o,$(BINFILES)) \
        $(CFILES:.c=.o) $(SFILES:.S=.o) $(CPPFILES:.cpp=.o) $(GFXFILES:.png=.o) $(FONTFILES:.pgf=.o)

PSP_LARGE_MEMORY = 1

VERSION_MAJOR :=  3
VERSION_MINOR :=  2
VERSION_MICRO :=  0

INCDIR   = ../libs/include ../libs/include/ogg ../libs/include/opus include include/menus
CFLAGS   = -O3 -G0 -Wall -Werror -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DVERSION_MICRO=$(VERSION_MICRO)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS  := $(CFLAGS)

BUILD_PRX = 1

LIBDIR  = ../libs/lib
LDFLAGS =
LIBS    = -lpsputility -lintrafont -lglib2d -lpspgum -lpspgu -lpsprtc -lpspvram \
          -lxmp-lite -lmpg123 -lvorbisfile -lvorbis -lopusfile -lopus -lFLAC -logg -lpspaudio \
          -larchive -lbz2 -llzma -lz \
          -lpspsdk -lpspctrl -lpsppower \
          -lpspreg -lpspusb -lpspusbstor -lpspusbdevice -lpspumd -lpspkubridge -lpspsystemctrl_user -lm

EXTRA_TARGETS    = EBOOT.PBP
PSP_EBOOT_TITLE  = CM File Manager PSP v$(VERSION_MAJOR).$(VERSION_MINOR)$(VERSION_MICRO)
PSP_EBOOT_ICON   = ../ICON0.PNG

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

%.o: %.png
	bin2o -i $< $@ $(addsuffix _png, $(basename $(notdir $<) ))

%.o: %.pgf
	bin2o -i $< $@ $(addsuffix _pgf, $(basename $(notdir $<) ))
