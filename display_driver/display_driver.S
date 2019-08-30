	.set noreorder

#include "pspstub.s"

	STUB_START	"display_driver",0x40010000,0x00040005
	STUB_FUNC	0x9C096DA0,display_driver_9C096DA0
	STUB_FUNC	0x193400C8,display_driver_193400C8
	STUB_FUNC	0xE3CA895B,display_driver_E3CA895B
	STUB_FUNC	0x105212A9,display_driver_105212A9
	STUB_END
