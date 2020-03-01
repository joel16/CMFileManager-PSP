	.set noreorder

#include "pspstub.s"

	STUB_START	"audio_driver",0x40090000,0x00010005
	STUB_FUNC	0x37516EBF,pspAudioSetFrequency
	STUB_END
