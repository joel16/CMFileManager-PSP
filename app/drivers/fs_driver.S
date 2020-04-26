	.set noreorder

#include "pspstub.s"

	STUB_START "fs_driver",0x40090000,0x00030005
	STUB_FUNC  0x69D6919D,pspOpenDir
	STUB_FUNC  0xA2A83466,pspReadDir
	STUB_FUNC  0x6F2145C0,pspCloseDir
	STUB_END
