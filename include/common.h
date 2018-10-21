#pragma once

#include <oslib/oslib.h>
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

#define BROWSE_STATE_SD     0
#define BROWSE_STATE_FLASH0 1
#define BROWSE_STATE_FLASH1 1
#define BROWSE_STATE_FLASH2 1
#define BROWSE_STATE_FLASH3 1

#define WHITE                 RGBA(255, 255, 255, 255)
#define BLACK_BG              RGBA(48, 48, 48, 255)
#define STATUS_BAR_LIGHT      RGBA(37, 79, 174, 255)
#define STATUS_BAR_DARK       RGBA(38, 50, 56, 255)
#define MENU_BAR_LIGHT        RGBA(51, 103, 214, 255)
#define MENU_BAR_DARK         RGBA(55, 71, 79, 255)
#define BLACK                 RGBA(0, 0, 0, 255)
#define SELECTOR_COLOUR_LIGHT RGBA(241, 241, 241, 255)
#define SELECTOR_COLOUR_DARK  RGBA(76, 76, 76, 255)
#define TITLE_COLOUR          RGBA(30, 136, 229, 255)
#define TITLE_COLOUR_DARK     RGBA(0, 150, 136, 255)
#define TEXT_MIN_COLOUR_LIGHT RGBA(32, 32, 32, 255)
#define TEXT_MIN_COLOUR_DARK  RGBA(185, 185, 185, 255)
#define BAR_COLOUR            RGBA(200, 200, 200, 255)

jmp_buf exitJmp;

OSL_FONT *font;

int MENU_STATE;
int BROWSE_STATE;

char cwd[512];
char root_path[10];
