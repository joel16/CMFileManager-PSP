#include "colours.h"
#include "config.h"

g2dColor BG_COLOUR, STATUS_BAR_COLOUR, MENU_BAR_COLOUR, SELECTOR_COLOUR, TITLE_COLOUR, TEXT_COLOUR;

namespace Colours {
    void Get(void) {
        BG_COLOUR = cfg.dark_theme? BLACK_BG : WHITE;
        STATUS_BAR_COLOUR = cfg.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT;
        MENU_BAR_COLOUR = cfg.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT;
        SELECTOR_COLOUR = cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT;
        TITLE_COLOUR = cfg.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR_LIGHT;
        TEXT_COLOUR = cfg.dark_theme? TEXT_COLOUR_DARK : TEXT_COLOUR_LIGHT;
    }
}
