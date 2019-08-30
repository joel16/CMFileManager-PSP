	.set noreorder

#include "pspstub.s"

	STUB_START	"audio_driver",0x40010000,0x00010005
	STUB_FUNC	0x37516EBF,audio_driver_37516EBF
	STUB_END
