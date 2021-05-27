#include "config.h"
#include "colours.h"
#include "fs.h"
#include "g2d.h"
#include "gui.h"
#include "textures.h"
#include "utils.h"

namespace GUI {
    enum SETTINGS_STATE {
        GENERAL_SETTINGS,
        FTP_SETTINGS,
        SORT_SETTINGS,
        ABOUT_SETTINGS
    };

    static SETTINGS_STATE settings_state = GENERAL_SETTINGS;
    static int selection = 0;
    static const int sel_dist = 44;

    static void DisplaySortSettings(void) {
        intraFontPrint(font, 40, 40, "Sorting Options");

        G2D::FontSetStyle(font, 1.0f, cfg.dark_theme? WHITE : BLACK, INTRAFONT_ALIGN_LEFT);
        intraFontPrint(font, 40, 72, "Alphabetical");
        intraFontPrint(font, 40, 86, "Sort alphabetically in ascending order.");

        intraFontPrint(font, 40, 116, "Alphabetical");
        intraFontPrint(font, 40, 130, "Sort alphabetically in descending order.");

        intraFontPrint(font, 40, 160, "Size");
        intraFontPrint(font, 40, 174, "Sort by size (largest first).");

        intraFontPrint(font, 40, 204, "Size");
        intraFontPrint(font, 40, 218, "Sort by size (smallest first).");

        G2D::DrawImage(cfg.sort == 0? (cfg.dark_theme? icon_radio_dark_on : icon_radio_on) : (cfg.dark_theme? icon_radio_dark_off : icon_radio_off), 425, 60);
        G2D::DrawImage(cfg.sort == 1? (cfg.dark_theme? icon_radio_dark_on : icon_radio_on) : (cfg.dark_theme? icon_radio_dark_off : icon_radio_off), 425, 104);
        G2D::DrawImage(cfg.sort == 2? (cfg.dark_theme? icon_radio_dark_on : icon_radio_on) : (cfg.dark_theme? icon_radio_dark_off : icon_radio_off), 425, 148);
        G2D::DrawImage(cfg.sort == 3? (cfg.dark_theme? icon_radio_dark_on : icon_radio_on) : (cfg.dark_theme? icon_radio_dark_off : icon_radio_off), 425, 192);
    }

    static void ControlSortSettings(MenuItem *item, int *ctrl) {
        if (Utils::IsButtonPressed(PSP_CTRL_ENTER)) {
            cfg.sort = selection;
            Config::Save(cfg);
            FS::GetDirList(cfg.cwd, item->entries);
        }
        else if (Utils::IsButtonPressed(PSP_CTRL_CANCEL)) {
            selection = 0;
            settings_state = GENERAL_SETTINGS;
        }

        Utils::SetBounds(&selection, 0, 3);
    }

    static void DisplayGeneralSettings(void) {
        intraFontPrint(font, 40, 40, "Settings");

        G2D::FontSetStyle(font, 1.0f, cfg.dark_theme? WHITE : BLACK, INTRAFONT_ALIGN_LEFT);
        intraFontPrint(font, 40, 72, "FTP connection");
        intraFontPrint(font, 40, 86, "Wireless connection");

        intraFontPrint(font, 40, 116, "Sorting options");
        intraFontPrint(font, 40, 130, "Select between various sorting options.");

        intraFontPrint(font, 40, 160, "Dark theme");
        intraFontPrint(font, 40, 174, "Enables dark theme mode.");

        intraFontPrint(font, 40, 204, "Developer options");
        intraFontPrint(font, 40, 218, "Enable logging and fs access to NAND.");

        intraFontPrint(font, 40, 248, "About");
        intraFontPrint(font, 40, 262, "Application and device info");

        if (cfg.dark_theme)
            G2D::DrawImage(cfg.dark_theme? icon_toggle_dark_on : icon_toggle_on, 415, 143);
        else
            G2D::DrawImage(icon_toggle_off, 415, 143);

        G2D::DrawImage(cfg.dev_options? (cfg.dark_theme? icon_toggle_dark_on : icon_toggle_on) : icon_toggle_off, 415, 187);
    }
    
    static void ControlGeneralSettings(MenuItem *item, int *ctrl) {
        if (Utils::IsButtonPressed(PSP_CTRL_ENTER)) {
            switch(selection) {
                case 1:
                    settings_state = SORT_SETTINGS;
                    selection = 0;
                    break;

                case 2:
                    cfg.dark_theme = !cfg.dark_theme;
                    Colours::Get();
                    Config::Save(cfg);
                    break;

                case 3:
                    cfg.dev_options = !cfg.dev_options;
                    Config::Save(cfg);
                    break;
                    
                default:
                    break;
            }
        }
        else if (Utils::IsButtonPressed(PSP_CTRL_CANCEL)) {
            selection = 0;
            item->state = MENU_STATE_FILEBROWSER;
        }

        Utils::SetBounds(&selection, 0, 4);
    }

    void DisplaySettings(MenuItem *item) {
        G2D::DrawRect(0, 18, 480, 34, MENU_BAR_COLOUR);
        G2D::DrawRect(0, 52, 480, 220, BG_COLOUR);
        G2D::DrawImage(icon_back, 5, 20);

        G2D::DrawRect(0, 52 + (selection * sel_dist), 480, sel_dist, SELECTOR_COLOUR);
        G2D::FontSetStyle(font, 1.0f, WHITE, INTRAFONT_ALIGN_LEFT);

        switch(settings_state) {
            case GENERAL_SETTINGS:
                DisplayGeneralSettings();
                break;
            
            case SORT_SETTINGS:
                DisplaySortSettings();
                break;

            default:
                break;
        }
    }

    void ControlSettings(MenuItem *item, int *ctrl) {
        if (*ctrl & PSP_CTRL_UP)
            selection--;
        else if (*ctrl & PSP_CTRL_DOWN)
            selection++;
        
        switch(settings_state) {
            case GENERAL_SETTINGS:
                ControlGeneralSettings(item, ctrl);
                break;
            
            case SORT_SETTINGS:
                ControlSortSettings(item, ctrl);
                break;

            default:
                break;
        }
    }
}
