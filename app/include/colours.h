#ifndef _CMFILEMANAGER_COLOURS_H_
#define _CMFILEMANAGER_COLOURS_H_

#include <glib2d.h>

constexpr g2dColor BLACK_BG = G2D_RGBA(48, 48, 48, 255);
constexpr g2dColor STATUS_BAR_LIGHT = G2D_RGBA(37, 79, 174, 255);
constexpr g2dColor STATUS_BAR_DARK = G2D_RGBA(38, 50, 56, 255);
constexpr g2dColor MENU_BAR_LIGHT = G2D_RGBA(51, 103, 214, 255);
constexpr g2dColor MENU_BAR_DARK = G2D_RGBA(55, 71, 79, 255);
constexpr g2dColor SELECTOR_COLOUR_LIGHT = G2D_RGBA(241, 241, 241, 255);
constexpr g2dColor SELECTOR_COLOUR_DARK = G2D_RGBA(76, 76, 76, 255);
constexpr g2dColor TITLE_COLOUR_LIGHT = G2D_RGBA(30, 136, 229, 255);
constexpr g2dColor TITLE_COLOUR_DARK = G2D_RGBA(0, 150, 136, 255);
constexpr g2dColor TEXT_COLOUR_LIGHT = G2D_RGBA(32, 32, 32, 255);
constexpr g2dColor TEXT_COLOUR_DARK = G2D_RGBA(185, 185, 185, 255);

extern g2dColor BG_COLOUR, STATUS_BAR_COLOUR, MENU_BAR_COLOUR, SELECTOR_COLOUR, TITLE_COLOUR, TEXT_COLOUR;

namespace Colours {
    void Get(void);
}

#endif
