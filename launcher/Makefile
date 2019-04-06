TARGET = CMFileManager

OBJS   = source/main.o

PSP_LARGE_MEMORY = 1

VERSION_MAJOR :=  2
VERSION_MINOR :=  3
VERSION_MICRO :=  0

INCDIR   = ../common
CFLAGS   = -G0 -Wall -Os -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DVERSION_MICRO=$(VERSION_MICRO)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS  := $(CFLAGS)

LIBDIR  = ../libs
LDFLAGS =
LIBS    = -lpspsdk -lpspsystemctrl_user

EXTRA_TARGETS    = EBOOT.PBP
PSP_EBOOT_TITLE  = CM File Manager PSP
PSP_EBOOT_ICON   = ../ICON0.PNG

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

%.o: %.png
	bin2o -i $< $@ $(addsuffix _png, $(basename $(notdir $<) ))

%.o: %.pgf
	bin2o -i $< $@ $(addsuffix _pgf, $(basename $(notdir $<) ))
