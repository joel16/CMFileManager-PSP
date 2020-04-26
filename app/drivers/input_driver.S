	.set noreorder

#include "pspstub.s"

	STUB_START "input_driver",0x40090000,0x00010005
	STUB_FUNC  0x358DD650,pspGetButtons
	STUB_END
