	.set noreorder

#include "pspstub.s"

	STUB_START "display_driver",0x40090000,0x00130005
	STUB_FUNC  0x19F94895,pspGetBrightness
	STUB_FUNC  0xB6551455,pspSetBrightness
	STUB_FUNC  0xAC18FDEC,pspDisplayEnable
	STUB_FUNC  0xDA2E4C8C,pspDisplayDisable
	STUB_END
