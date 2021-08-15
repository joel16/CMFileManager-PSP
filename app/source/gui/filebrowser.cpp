#include <algorithm>
#include <pspctrl.h>

#include "archive_helper.h"
#include "audioplayer.h"
#include "config.h"
#include "fs.h"
#include "colours.h"
#include "g2d.h"
#include "game_launcher.h"
#include "gui.h"
#include "textures.h"
#include "texteditor.h"
#include "utils.h"

namespace GUI {
    static const int sel_dist = 20;
    static const int start_y = 52;
    static const u32 max_entries = 11;
    static int start = 0;
    
    static const std::string empty_dir = "This is an empty directory";
    
    void DisplayFileBrowser(MenuItem *item) {
        G2D::FontSetStyle(font, 1.0f, WHITE, INTRAFONT_ALIGN_LEFT);
        float height = G2D::GetTextHeight(font);
        intraFontPrintf(font, 40, 36, cfg.cwd.length() > 42? "%.42s..." : "%s", cfg.cwd.c_str());
        G2D::DrawRect(40, 43, 400, 3, SELECTOR_COLOUR);

        if ((device == BROWSE_STATE_INTERNAL) || (device == BROWSE_STATE_EXTERNAL)) {
            float fill = (static_cast<double>(item->used_storage)/static_cast<double>(item->total_storage)) * 400.f;
            G2D::DrawRect(40, 43, fill, 3, TITLE_COLOUR);
        }
        
        if (item->entries.empty()) {
            G2D::FontSetStyle(font, 1.0f, cfg.dark_theme? WHITE : BLACK, INTRAFONT_ALIGN_CENTER);
            G2D::DrawText(240, 136, empty_dir.c_str());
        }

        for (u32 i = start; i < item->entries.size(); i++) {
            std::string filename = item->entries[i].d_name;

            if (i == static_cast<u32>(item->selected))
                G2D::DrawRect(0, start_y + (sel_dist * (i - start)), 480, sel_dist, SELECTOR_COLOUR);

            if ((item->checked[i]) && (!item->checked_cwd.compare(cfg.cwd)))
                G2D::DrawImageScale(icon_check[cfg.dark_theme], 0, start_y + (sel_dist * (i - start)), 18.f, 18.f);
            else
                G2D::DrawImageScale(icon_uncheck[cfg.dark_theme], 0, start_y + (sel_dist * (i - start)), 18.f, 18.f);

            FileType file_type = FS::GetFileType(filename);
            if (FIO_S_ISDIR(item->entries[i].d_stat.st_mode))
                G2D::DrawImageScale(icon_dir[cfg.dark_theme], 20, start_y + (sel_dist * (i - start)), 18.f, 18.f);
            else
                G2D::DrawImageScale(file_icons[file_type], 20, start_y + (sel_dist * (i - start)), 18.f, 18.f);

            G2D::FontSetStyle(font, 1.0f, cfg.dark_theme? WHITE : BLACK, INTRAFONT_ALIGN_LEFT);
            intraFontPrintf(font, 45, start_y + 10 + ((sel_dist - height) / 2) + (i - start) * sel_dist, filename.length() > 42? "%.42s..." : "%s", filename.c_str());
        }
    }

    void ControlFileBrowser(MenuItem *item, int *ctrl) {
        u32 size = (item->entries.size() - 1);
        Utils::SetBounds(&item->selected, 0, size);

        if (*ctrl & PSP_CTRL_UP) {
            item->selected--;
            if (item->selected < 0)
                item->selected = size;

            if (size < max_entries)
                start = 0;
            else if (start > item->selected)
                start--;
            else if ((static_cast<u32>(item->selected) == size) && (size > (max_entries - 1)))
                start = size - (max_entries - 1);
        }
        else if (*ctrl & PSP_CTRL_DOWN) {
            item->selected++;
            if(static_cast<u32>(item->selected) > size)
                item->selected = 0;

            if ((static_cast<u32>(item->selected) > (start + (max_entries - 1))) && ((start + (max_entries - 1)) < size))
                start++;
            if (item->selected == 0)
                start = 0;
        }

        if (Utils::IsButtonPressed(PSP_CTRL_LEFT)) {
            item->selected = 0;
            start = 0;
        }
        else if (Utils::IsButtonPressed(PSP_CTRL_RIGHT)) {
            item->selected = item->entries.size() - 1;
            if ((item->entries.size() - 1) > max_entries)
                start = size - (max_entries - 1);
        }

        if (Utils::IsButtonPressed(PSP_CTRL_ENTER)) {
            if (FIO_S_ISDIR(item->entries[item->selected].d_stat.st_mode)) {
                if (item->entries.size() != 0) {
                    if (R_SUCCEEDED(FS::ChangeDirNext(item->entries[item->selected].d_name, item->entries))) {
                        start = 0;
                        // Make a copy before resizing our vector.
                        if ((item->checked_count > 1) && (item->checked_copy.empty()))
                            item->checked_copy = item->checked;
                        
                        item->checked.resize(item->entries.size());
                        item->selected = 0;
                    }
                }
            }
            else {
                std::string path = FS::BuildPath(cfg.cwd, item->entries[item->selected].d_name);
                FileType file_type = FS::GetFileType(item->entries[item->selected].d_name);
                
                switch(file_type) {
                    case FileTypeApp:
                        GameLauncher::DisplayLauncher(path);
                        break;
                    
                    case FileTypeAudio:
                        AudioPlayer::Play(path);
                        break;

                    case FileTypeArchive:
                        if (R_SUCCEEDED(ArchiveHelper::Extract(path))) {
                            FS::GetDirList(cfg.cwd, item->entries);
                            GUI::ResetCheckbox(item);
                        }
                        break;
                    
                    case FileTypeImage:
                        item->texture = Textures::LoadImage(path.c_str());
                        if (item->texture)
                            item->state = MENU_STATE_IMAGEVIEWER;
                        break;

                    case FileTypeText:
                        TextViewer::Edit(path);
                        break;
                    
                    default:
                        break;
                }
            }
        }
        else if (Utils::IsButtonPressed(PSP_CTRL_CANCEL)) {
            if (R_SUCCEEDED(FS::ChangeDirPrev(item->entries))) {
                // Make a copy before resizing our vector.
                if (item->checked_count > 1)
                    item->checked_copy = item->checked;
                    
                item->checked.resize(item->entries.size());
                item->selected = 0;
                start = 0;
            }
        }
        else if (Utils::IsButtonPressed(PSP_CTRL_SQUARE)) {
            if ((!item->checked_cwd.empty()) && (item->checked_cwd.compare(cfg.cwd) != 0))
                GUI::ResetCheckbox(item);
                
            item->checked_cwd = cfg.cwd;
            item->checked[item->selected] = !item->checked[item->selected];
            item->checked_count = std::count(item->checked.begin(), item->checked.end(), true);
        }
        else if (Utils::IsButtonPressed(PSP_CTRL_TRIANGLE))
            item->state = MENU_STATE_OPTIONS;

        if (Utils::IsButtonPressed(PSP_CTRL_SELECT))
            item->state = MENU_STATE_MENUBAR;
    }
}
