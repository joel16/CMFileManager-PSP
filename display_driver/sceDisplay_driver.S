	.set noreorder

#include "pspimport.s"

	IMPORT_START "sceDisplay_driver",0x00010000
	IMPORT_FUNC	"sceDisplay_driver",0x1CB8CB47,sceDisplayGetBrightness371
	IMPORT_FUNC	"sceDisplay_driver",0x776ADFDB,sceDisplaySetBrightness371
	IMPORT_FUNC	"sceDisplay_driver",0x946155FD,sceDisplayEnable371
	IMPORT_FUNC	"sceDisplay_driver",0x32B67781,sceDisplayDisable371

	IMPORT_FUNC	"sceDisplay_driver",0x31C4BAA8,sceDisplayGetBrightness
	IMPORT_FUNC	"sceDisplay_driver",0x9E3C6DC6,sceDisplaySetBrightness
	IMPORT_FUNC	"sceDisplay_driver",0x432D133F,sceDisplayEnable
	IMPORT_FUNC	"sceDisplay_driver",0x681EE6A7,sceDisplayDisable
