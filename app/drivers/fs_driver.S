	.set noreorder

#include "pspstub.s"

	STUB_START	"fs_driver",0x40090000,0x00030005
	STUB_FUNC	0xF848B45F,fsOpenDir
	STUB_FUNC	0xDAD85041,fsReadDir
	STUB_FUNC	0xE91DD61A,fsCloseDir
	STUB_END
