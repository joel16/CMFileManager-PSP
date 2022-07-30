TARGET = module_driver
OBJS = module_driver.o exports.o ModuleMgrForKernel.o

PRX_EXPORTS = exports.exp

# Use the kernel's small inbuilt libc
USE_KERNEL_LIBC = 1
# Use only kernel libraries
USE_KERNEL_LIBS = 1

CFLAGS = -Os -G0 -Wall -fno-builtin-printf
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS = -nostartfiles

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

all:
	psp-build-exports -s $(PRX_EXPORTS)
	mv module_driver.S "../app/drivers/"
	mv module_driver.prx "../app/data/"
