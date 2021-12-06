#include "config.h"
#include "fs.h"
#include "colours.h"
#include "g2d.h"
#include "gui.h"
#include "log.h"
#include "textures.h"
#include "utils.h"

namespace GUI {
    void DisplayFileProperties(MenuItem *item) {
        G2D::DrawRect(0, 18, 480, 254, G2D_RGBA(0, 0, 0, cfg.dark_theme? 50: 80));
        G2D::DrawImage(properties_dialog[cfg.dark_theme], ((480 - (properties_dialog[0]->w)) / 2), ((272 - (properties_dialog[0]->h)) / 2));
        G2D::FontSetStyle(font, 1.0f, TITLE_COLOUR, INTRAFONT_ALIGN_LEFT);

        G2D::DrawText(((480 - (properties_dialog[0]->w)) / 2) + 10, ((272 - (properties_dialog[0]->h)) / 2) + 20, "Properties");

        int ok_width = intraFontMeasureText(font, "OK");
        G2D::DrawRect((340 - (ok_width)) - 5, (220 - (font->texYSize - 15)) - 5, ok_width + 10, (font->texYSize - 5) + 10, SELECTOR_COLOUR);
        G2D::DrawText(340 - (ok_width), (232 - (font->texYSize - 15)) - 3, "OK");

        G2D::FontSetStyle(font, 1.0f, TEXT_COLOUR, INTRAFONT_ALIGN_LEFT);
        intraFontPrintf(font, 140, 74, std::string(item->entries[item->selected].d_name).length() > 14? "Name: %.14s..." : "%s", 
            item->entries[item->selected].d_name);
        
        if (!(FIO_S_ISDIR(item->entries[item->selected].d_stat.st_mode))) {
            char size[16];
            Utils::GetSizeString(size, item->entries[item->selected].d_stat.st_size);
            intraFontPrintf(font, 140, 92, "Size: %s", size);
            intraFontPrintf(font, 140, 110, "Created: %s", FS::GetFileTimestamp(item->entries[item->selected].d_stat, FileCreatedTime));
            intraFontPrintf(font, 140, 128, "Accessed: %s", FS::GetFileTimestamp(item->entries[item->selected].d_stat, FileAccessedTime));
            intraFontPrintf(font, 140, 146, "Modified: %s", FS::GetFileTimestamp(item->entries[item->selected].d_stat, FileModifiedTime));
            intraFontPrintf(font, 140, 164, "Perms: %s", FS::GetFilePermission(item->entries[item->selected].d_stat));
        }
        else {
            intraFontPrintf(font, 140, 92, "Created: %s", FS::GetFileTimestamp(item->entries[item->selected].d_stat, FileCreatedTime));
            intraFontPrintf(font, 140, 110, "Accessed: %s", FS::GetFileTimestamp(item->entries[item->selected].d_stat, FileAccessedTime));
            intraFontPrintf(font, 140, 128, "Modified: %s", FS::GetFileTimestamp(item->entries[item->selected].d_stat, FileModifiedTime));
            intraFontPrintf(font, 140, 146, "Perms: %s", FS::GetFilePermission(item->entries[item->selected].d_stat));
        }
    }

    void ControlFileProperties(MenuItem *item) {
        if ((Utils::IsButtonPressed(PSP_CTRL_ENTER)) || (Utils::IsButtonPressed(PSP_CTRL_CANCEL)))
            item->state = MENU_STATE_OPTIONS;
    }
}
