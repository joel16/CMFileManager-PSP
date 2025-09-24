#include "colours.h"
#include "config.h"
#include "fs.h"
#include "g2d.h"
#include "gui.h"
#include "textures.h"
#include "utils.h"

namespace Options {
    void Delete(MenuItem &item, int *selection) {
        int ret = 0;
        
        if ((item.checked_count > 1) && (!item.checked_cwd.compare(cfg.cwd))) {
            for (u32 i = 0; i < item.checked.size(); i++) {
                if (item.checked.at(i)) {
                    if (R_FAILED(ret = FS::Delete(item.entries[i]))) {
                        FS::GetDirList(cfg.cwd, item.entries);
                        GUI::ResetCheckbox(item);
                        break;
                    }
                }
            }
        }
        else {
            ret = FS::Delete(item.entries[item.selected]);
        }
        
        if (R_SUCCEEDED(ret)) {
            FS::GetDirList(cfg.cwd, item.entries);
            GUI::ResetCheckbox(item);
        }
        
        GUI::GetStorageSize(item);
        *selection = 0;
        item.selected = 0;
        item.state = MENU_STATE_FILEBROWSER;
    }
}

namespace GUI {
    static int selection = 0;
    static const char *prompt = "Do you wish to continue?";

    void DisplayDeleteOptions(void) {
        G2D::DrawRect(0, 18, 480, 254, G2D_RGBA(0, 0, 0, cfg.dark_theme? 50: 80));
        G2D::DrawImage(dialog[cfg.dark_theme], ((480 - (dialog[0]->w)) / 2), ((272 - (dialog[0]->h)) / 2));
        G2D::FontSetStyle(1.f, TITLE_COLOUR, INTRAFONT_ALIGN_LEFT);
        G2D::DrawText(((480 - (dialog[0]->w)) / 2) + 10, ((272 - (dialog[0]->h)) / 2) + 20, "Delete");

        int confirmWidth = intraFontMeasureText(fonts[FONT_DEFAULT], "YES");
        int cancelWidth = intraFontMeasureText(fonts[FONT_DEFAULT], "NO");
        
        if (selection == 0) {
            G2D::DrawRect((364 - cancelWidth) - 5, (180 - (fonts[FONT_DEFAULT]->texYSize - 15)) - 5, cancelWidth + 10, (fonts[FONT_DEFAULT]->texYSize - 5) + 10, SELECTOR_COLOUR);
        }
        else {
            G2D::DrawRect((409 - (confirmWidth)) - 5, (180 - (fonts[FONT_DEFAULT]->texYSize - 15)) - 5, confirmWidth + 10, (fonts[FONT_DEFAULT]->texYSize - 5) + 10, SELECTOR_COLOUR);
        }
        
        G2D::DrawText(409 - (confirmWidth), (192 - (fonts[FONT_DEFAULT]->texYSize - 15)) - 3, "YES");
        G2D::DrawText(364 - cancelWidth, (192 - (fonts[FONT_DEFAULT]->texYSize - 15)) - 3, "NO");

        int promptWidth = intraFontMeasureText(fonts[FONT_DEFAULT], prompt);
        G2D::FontSetStyle(1.f, TEXT_COLOUR, INTRAFONT_ALIGN_LEFT);
        G2D::DrawText(((480 - (promptWidth)) / 2), ((272 - (dialog[0]->h)) / 2) + 60, prompt);
    }

    void ControlDeleteOptions(MenuItem &item, int &ctrl) {
        if (ctrl & PSP_CTRL_RIGHT) {
            selection++;
        }
        else if (ctrl & PSP_CTRL_LEFT) {
            selection--;
        }

        if (Utils::IsButtonPressed(PSP_CTRL_ENTER)) {
            if (selection == 1) {
                Options::Delete(item, &selection);
            }
            else {
                item.state = MENU_STATE_OPTIONS;
            }

        }
        else if (Utils::IsButtonPressed(PSP_CTRL_CANCEL)) {
            item.state = MENU_STATE_OPTIONS;
        }
        
        Utils::SetBounds(selection, 0, 1);
    }
}
