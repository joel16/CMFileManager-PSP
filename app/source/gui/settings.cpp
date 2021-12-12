#include "colours.h"
#include "config.h"
#include "fs.h"
#include "g2d.h"
#include "gui.h"
#include "kernel_functions.h"
#include "net.h"
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
    static char ftp_text[36];
    static bool screen_disabled = false;

    static void DisplayFTPSettings(void) {
        G2D::DrawRect(0, 18, 480, 254, G2D_RGBA(0, 0, 0, cfg.dark_theme? 50: 80));
        G2D::DrawImage(dialog[cfg.dark_theme], ((480 - (dialog[0]->w)) / 2), ((272 - (dialog[0]->h)) / 2));
        G2D::FontSetStyle(font, 1.0f, TITLE_COLOUR, INTRAFONT_ALIGN_LEFT);
        G2D::DrawText(((480 - (dialog[0]->w)) / 2) + 10, ((272 - (dialog[0]->h)) / 2) + 20, "FTP");

        int ok_width = intraFontMeasureText(font, "OK");
        G2D::DrawRect((409 - (ok_width)) - 5, (180 - (font->texYSize - 15)) - 5, ok_width + 10, (font->texYSize - 5) + 10, SELECTOR_COLOUR);
        G2D::DrawText(409 - (ok_width), (192 - (font->texYSize - 15)) - 3, "OK");
        
        int text_width = intraFontMeasureText(font, ftp_text);
        G2D::FontSetStyle(font, 1.0f, TEXT_COLOUR, INTRAFONT_ALIGN_LEFT);
        G2D::DrawText(((480 - (text_width)) / 2), ((272 - (dialog[0]->h)) / 2) + 60, ftp_text);
    }

    static void ControlFTPSettings(void) {
        if (Utils::IsButtonPressed(PSP_CTRL_CANCEL)) {
            Net::ExitFTP();
            settings_state = GENERAL_SETTINGS;
            
            if (screen_disabled)
                pspDisplayEnable();
        }

        if (Utils::IsButtonPressed(PSP_CTRL_SELECT)) {
            screen_disabled = !screen_disabled;

            if (screen_disabled)
                pspDisplayDisable();
            else
                pspDisplayEnable();
        }

        Utils::SetBounds(selection, 0, 0);
    }

    static void DisplaySortSettings(void) {
        G2D::DrawText(40, 40, "Sorting Options");

        G2D::FontSetStyle(font, 1.0f, cfg.dark_theme? WHITE : BLACK, INTRAFONT_ALIGN_LEFT);
        G2D::DrawText(40, 72, "Alphabetical");
        G2D::DrawText(40, 86, "Sort alphabetically in ascending order.");

        G2D::DrawText(40, 116, "Alphabetical");
        G2D::DrawText(40, 130, "Sort alphabetically in descending order.");

        G2D::DrawText(40, 160, "Size");
        G2D::DrawText(40, 174, "Sort by size (largest first).");

        G2D::DrawText(40, 204, "Size");
        G2D::DrawText(40, 218, "Sort by size (smallest first).");

        G2D::DrawImage(cfg.sort == 0? icon_radio_on[cfg.dark_theme] : icon_radio_off[cfg.dark_theme], 425, 60);
        G2D::DrawImage(cfg.sort == 1? icon_radio_on[cfg.dark_theme] : icon_radio_off[cfg.dark_theme], 425, 104);
        G2D::DrawImage(cfg.sort == 2? icon_radio_on[cfg.dark_theme] : icon_radio_off[cfg.dark_theme], 425, 148);
        G2D::DrawImage(cfg.sort == 3? icon_radio_on[cfg.dark_theme] : icon_radio_off[cfg.dark_theme], 425, 192);
    }

    static void ControlSortSettings(MenuItem *item) {
        if (Utils::IsButtonPressed(PSP_CTRL_ENTER)) {
            cfg.sort = selection;
            Config::Save(cfg);
            FS::GetDirList(cfg.cwd, item->entries);
        }
        else if (Utils::IsButtonPressed(PSP_CTRL_CANCEL)) {
            selection = 0;
            settings_state = GENERAL_SETTINGS;
        }

        Utils::SetBounds(selection, 0, 3);
    }

    static void DisplayAboutSettings(void) {
        G2D::DrawRect(0, 18, 480, 254, G2D_RGBA(0, 0, 0, cfg.dark_theme? 50: 80));
        G2D::DrawImage(dialog[cfg.dark_theme], ((480 - (dialog[0]->w)) / 2), ((272 - (dialog[0]->h)) / 2));
        G2D::FontSetStyle(font, 1.0f, TITLE_COLOUR, INTRAFONT_ALIGN_LEFT);
        G2D::DrawText(((480 - (dialog[0]->w)) / 2) + 10, ((272 - (dialog[0]->h)) / 2) + 20, "About");

        int ok_width = intraFontMeasureText(font, "OK");
        G2D::DrawRect((409 - (ok_width)) - 5, (180 - (font->texYSize - 15)) - 5, ok_width + 10, (font->texYSize - 5) + 10, SELECTOR_COLOUR);
        G2D::DrawText(409 - (ok_width), (192 - (font->texYSize - 15)) - 3, "OK");
        
        G2D::FontSetStyle(font, 1.0f, TEXT_COLOUR, INTRAFONT_ALIGN_LEFT);
        int version_width = intraFontMeasureText(font, "CMFileManager-PSP version: v4.0.0");
        intraFontPrintf(font, ((480 - (version_width)) / 2), ((272 - (dialog[0]->h)) / 2) + 50, "CMFileManager-PSP version: v%d.%d.%d", 
            VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);

        int author_width = intraFontMeasureText(font, "Author: Joel16");
        G2D::DrawText(((480 - (author_width)) / 2), ((272 - (dialog[0]->h)) / 2) + 68, "Author: Joel16");
    }

    static void ControlAboutSettings(void) {
        if ((Utils::IsButtonPressed(PSP_CTRL_ENTER)) || (Utils::IsButtonPressed(PSP_CTRL_CANCEL)))
            settings_state = GENERAL_SETTINGS;
        
        Utils::SetBounds(selection, 4, 4);
    }

    static void DisplayGeneralSettings(void) {
        G2D::DrawText(40, 40, "Settings");

        G2D::FontSetStyle(font, 1.0f, cfg.dark_theme? WHITE : BLACK, INTRAFONT_ALIGN_LEFT);

        G2D::DrawImage(ftp_icon[cfg.dark_theme], 15, 59);
        G2D::DrawText(60, 72, "FTP connection");
        G2D::DrawText(60, 86, "Wireless connection");

        G2D::DrawImage(sort_icon[cfg.dark_theme], 15, 103);
        G2D::DrawText(60, 116, "Sorting options");
        G2D::DrawText(60, 130, "Select between various sorting options.");

        G2D::DrawImage(dark_theme_icon[cfg.dark_theme], 15, 147);
        G2D::DrawText(60, 160, "Dark theme");
        G2D::DrawText(60, 174, "Enables dark theme mode.");

        G2D::DrawImage(dev_options_icon[cfg.dark_theme], 15, 191);
        G2D::DrawText(60, 204, "Developer options");
        G2D::DrawText(60, 218, "Enable logging and fs access to NAND.");

        G2D::DrawImage(about_icon[cfg.dark_theme], 15, 235);
        G2D::DrawText(60, 248, "About");
        G2D::DrawText(60, 262, "Application and device info");

        if (cfg.dark_theme)
            G2D::DrawImage(icon_toggle_on[cfg.dark_theme], 415, 143);
        else
            G2D::DrawImage(icon_toggle_off, 415, 143);

        G2D::DrawImage(cfg.dev_options? icon_toggle_on[cfg.dark_theme] : icon_toggle_off, 415, 187);
    }
    
    static void ControlGeneralSettings(MenuItem *item, int *ctrl) {
        if (Utils::IsButtonPressed(PSP_CTRL_ENTER)) {
            switch(selection) {
                case 0:
                    Net::InitFTP(ftp_text);
                    settings_state = FTP_SETTINGS;
                    break;
                
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
                    
                case 4:
                    settings_state = ABOUT_SETTINGS;
                    break;
            }
        }
        else if (Utils::IsButtonPressed(PSP_CTRL_CANCEL)) {
            selection = 0;
            item->state = MENU_STATE_FILEBROWSER;
        }

        Utils::SetBounds(selection, 0, 4);
    }

    void DisplaySettings(MenuItem *item) {
        G2D::DrawRect(0, 18, 480, 34, MENU_BAR_COLOUR);
        G2D::DrawRect(0, 52, 480, 220, BG_COLOUR);
        G2D::DrawImage(icon_back, 5, 20);

        G2D::DrawRect(0, 52 + (selection * sel_dist), 480, sel_dist, SELECTOR_COLOUR);
        G2D::FontSetStyle(font, 1.0f, WHITE, INTRAFONT_ALIGN_LEFT);

        switch(settings_state) {
            case GENERAL_SETTINGS:
                GUI::DisplayGeneralSettings();
                break;

            case FTP_SETTINGS:
                GUI::DisplayGeneralSettings();
                GUI::DisplayFTPSettings();
                break;
            
            case SORT_SETTINGS:
                GUI::DisplaySortSettings();
                break;
                
            case ABOUT_SETTINGS:
                GUI::DisplayGeneralSettings();
                GUI::DisplayAboutSettings();
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
                GUI::ControlGeneralSettings(item, ctrl);
                break;

            case FTP_SETTINGS:
                GUI::ControlFTPSettings();
                break;
            
            case SORT_SETTINGS:
                GUI::ControlSortSettings(item);
                break;
                
            case ABOUT_SETTINGS:
                GUI::ControlAboutSettings();
                break;
        }
    }
}
