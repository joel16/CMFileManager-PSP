	.set noreorder

#include "pspstub.s"

	STUB_START	"impose_driver",0x40090000,0x00070005
	STUB_FUNC	0x6AFB7691,pspGetVolume
	STUB_FUNC	0x57DE82E8,pspSetVolume
	STUB_FUNC	0x60390DAF,pspGetMute
	STUB_FUNC	0x5A376E5A,pspSetMute
	STUB_FUNC	0xBA4995BF,pspGetEqualizerMode
	STUB_FUNC	0x55BF8829,pspSetEqualizerMode
	STUB_FUNC	0x45F32DD0,pspSetHomePopup
	STUB_END
