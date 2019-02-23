#pragma once

#include <glib2d.h>
#include <intraFont.h>
#include <psptypes.h>

#include <stdbool.h>
#include <setjmp.h>

#define MAX_FILES 1024
#define FILES_PER_PAGE 5

#define MENU_STATE_HOME        0
#define MENU_STATE_MENUBAR     1
#define MENU_STATE_FILEOPTIONS 2
#define MENU_STATE_SETTINGS    3
#define MENU_STATE_FTP         4
#define MENU_STATE_SORT        5
#define MENU_STATE_DELETE      6
#define MENU_STATE_PROPERTIES  7
#define MENU_STATE_UPDATE      8
#define MENU_STATE_UPDATE_2    9
#define MENU_STATE_ABOUT       10

#define BROWSE_STATE_INTERNAL 0
#define BROWSE_STATE_SD       1
#define BROWSE_STATE_FLASH0   2
#define BROWSE_STATE_FLASH1   3
#define BROWSE_STATE_UMD      4
//#define BROWSE_STATE_FLASH2   3
//#define BROWSE_STATE_FLASH3   4

#define WHITE                 G2D_RGBA(255, 255, 255, 255)
#define BLACK_BG              G2D_RGBA(48, 48, 48, 255)
#define STATUS_BAR_LIGHT      G2D_RGBA(37, 79, 174, 255)
#define STATUS_BAR_DARK       G2D_RGBA(38, 50, 56, 255)
#define MENU_BAR_LIGHT        G2D_RGBA(51, 103, 214, 255)
#define MENU_BAR_DARK         G2D_RGBA(55, 71, 79, 255)
#define BLACK                 G2D_RGBA(0, 0, 0, 255)
#define SELECTOR_COLOUR_LIGHT G2D_RGBA(241, 241, 241, 255)
#define SELECTOR_COLOUR_DARK  G2D_RGBA(76, 76, 76, 255)
#define TITLE_COLOUR          G2D_RGBA(30, 136, 229, 255)
#define TITLE_COLOUR_DARK     G2D_RGBA(0, 150, 136, 255)
#define TEXT_MIN_COLOUR_LIGHT G2D_RGBA(32, 32, 32, 255)
#define TEXT_MIN_COLOUR_DARK  G2D_RGBA(185, 185, 185, 255)
#define BAR_COLOUR            G2D_RGBA(200, 200, 200, 255)

jmp_buf exitJmp;

int MENU_STATE;
int BROWSE_STATE;

intraFont *font;

char cwd[512];
char root_path[10];
bool is_ms_inserted;
bool is_psp_go;
