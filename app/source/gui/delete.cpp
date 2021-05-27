#include "config.h"
#include "fs.h"
#include "colours.h"
#include "g2d.h"
#include "gui.h"
#include "log.h"
#include "textures.h"
#include "utils.h"

namespace Options {
    void Delete(MenuItem *item, int *selection) {
        int ret = 0;
        Log::CloseHandle();
        
        if ((item->checked_count > 1) && (!item->checked_cwd.compare(cfg.cwd))) {
            for (u32 i = 0; i < item->checked.size(); i++) {
                if (item->checked.at(i)) {
                    if (R_FAILED(ret = FS::Delete(&item->entries[i]))) {
                        FS::GetDirList(cfg.cwd, item->entries);
                        GUI::ResetCheckbox(item);
                        break;
                    }
                }
            }
        }
        else
            ret = FS::Delete(&item->entries[item->selected]);
        
        if (R_SUCCEEDED(ret)) {
            FS::GetDirList(cfg.cwd, item->entries);
            GUI::ResetCheckbox(item);
        }
        
        GUI::GetStorageSize(item);
        Log::OpenHande();
        *selection = 0;
        item->selected = 0;
        item->state = MENU_STATE_FILEBROWSER;
    }
}

namespace GUI {
    static int selection = 0;
    static const std::string prompt = "Do you wish to continue?";

    void DisplayDeleteOptions(MenuItem *item) {
        G2D::DrawRect(0, 18, 480, 254, G2D_RGBA(0, 0, 0, cfg.dark_theme? 50: 80));
        G2D::DrawImage(cfg.dark_theme? dialog_dark : dialog, ((480 - (dialog->w)) / 2), ((272 - (dialog->h)) / 2));
        G2D::FontSetStyle(font, 1.0f, TITLE_COLOUR, INTRAFONT_ALIGN_LEFT);
        intraFontPrint(font, ((480 - (dialog->w)) / 2) + 10, ((272 - (dialog->h)) / 2) + 20, "Delete");

        int prompt_width = intraFontMeasureText(font, prompt.c_str());
        G2D::FontSetStyle(font, 1.0f, TEXT_COLOUR, INTRAFONT_ALIGN_LEFT);
        intraFontPrint(font, ((480 - (prompt_width)) / 2), ((272 - (dialog->h)) / 2) + 60, prompt.c_str());
        
        int confirm_width = intraFontMeasureText(font, "YES");
        int cancel_width = intraFontMeasureText(font, "NO");
        
        if (selection == 0)
            G2D::DrawRect((364 - cancel_width) - 5, (180 - (font->texYSize - 15)) - 5, cancel_width + 10, (font->texYSize - 5) + 10, SELECTOR_COLOUR);
        else
            G2D::DrawRect((409 - (confirm_width)) - 5, (180 - (font->texYSize - 15)) - 5, confirm_width + 10, (font->texYSize - 5) + 10, SELECTOR_COLOUR);

        G2D::FontSetStyle(font, 1.0f, TITLE_COLOUR, INTRAFONT_ALIGN_LEFT);
        intraFontPrint(font, 409 - (confirm_width), (192 - (font->texYSize - 15)) - 3, "YES");
        intraFontPrint(font, 364 - cancel_width, (192 - (font->texYSize - 15)) - 3, "NO");
    }

    void ControlDeleteOptions(MenuItem *item, int *ctrl) {
        if (*ctrl & PSP_CTRL_RIGHT)
            selection++;
        else if (*ctrl & PSP_CTRL_LEFT)
            selection--;

        if (Utils::IsButtonPressed(PSP_CTRL_ENTER)) {
            if (selection == 1)
                Options::Delete(item, &selection);
            else
                item->state = MENU_STATE_OPTIONS;

        }
        else if (Utils::IsButtonPressed(PSP_CTRL_CANCEL))
            item->state = MENU_STATE_OPTIONS;
        
        Utils::SetBounds(&selection, 0, 1);
    }
}
