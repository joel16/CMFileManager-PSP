#ifndef CMFILEMANAGER_COMMON_H
#define CMFILEMANAGER_COMMON_H

#include <glib2d.h>
#include <intraFont.h>
#include <psptypes.h>

#include <stdbool.h>
#include <setjmp.h>

enum MENU_STATES {
	MENU_STATE_HOME = 0,
	MENU_STATE_MENUBAR = 1,
	MENU_STATE_FILEOPTIONS = 2,
	MENU_STATE_SETTINGS = 3,
	MENU_STATE_FTP = 4,
	MENU_STATE_SORT = 5,
	MENU_STATE_DELETE = 6,
	MENU_STATE_PROPERTIES = 7,
	MENU_STATE_UPDATE = 8,
	MENU_STATE_UPDATE_2 = 9,
	MENU_STATE_ABOUT = 10
};

enum BROWSE_STATES {
	BROWSE_STATE_INTERNAL = 0,
	BROWSE_STATE_SD = 1,
	BROWSE_STATE_FLASH0 = 2,
	BROWSE_STATE_FLASH1 = 3,
	BROWSE_STATE_FLASH2 = 4,
	BROWSE_STATE_FLASH3 = 5,
	BROWSE_STATE_UMD = 6
};

// E1828

#define BLACK_BG              G2D_RGBA(48, 48, 48, 255)
#define STATUS_BAR_LIGHT      G2D_RGBA(37, 79, 174, 255)
#define STATUS_BAR_DARK       G2D_RGBA(38, 50, 56, 255)
#define MENU_BAR_LIGHT        G2D_RGBA(51, 103, 214, 255)
#define MENU_BAR_DARK         G2D_RGBA(55, 71, 79, 255)
#define SELECTOR_COLOUR_LIGHT G2D_RGBA(241, 241, 241, 255)
#define SELECTOR_COLOUR_DARK  G2D_RGBA(76, 76, 76, 255)
#define TITLE_COLOUR          G2D_RGBA(30, 136, 229, 255)
#define TITLE_COLOUR_DARK     G2D_RGBA(0, 150, 136, 255)
#define TEXT_MIN_COLOUR_LIGHT G2D_RGBA(32, 32, 32, 255)
#define TEXT_MIN_COLOUR_DARK  G2D_RGBA(185, 185, 185, 255)
#define BAR_COLOUR            G2D_RGBA(200, 200, 200, 255)

extern jmp_buf exitJmp;

extern int MENU_STATE;
extern int BROWSE_STATE;

extern intraFont *font, *jpn_font, *chn_font, *kor_font, *sym_font;

extern char cwd[512];
extern char root_path[10];
extern char initial_cwd[128];

extern bool is_ms_inserted;
extern bool is_psp_go;

#endif
