	.set noreorder

#include "pspstub.s"

	STUB_START	"display_driver",0x40090000,0x00040005
	STUB_FUNC	0x9C096DA0,pspGetBrightness
	STUB_FUNC	0x193400C8,pspSetBrightness
	STUB_FUNC	0xE3CA895B,pspDisplayEnable
	STUB_FUNC	0x105212A9,pspDisplayDisable
	STUB_END
