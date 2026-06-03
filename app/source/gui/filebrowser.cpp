#include <algorithm>
#include <cstring>
#include <pspctrl.h>

#include "archive_helper.h"
#include "audioplayer.h"
#include "colours.h"
#include "config.h"
#include "fs.h"
#include "g2d.h"
#include "game_launcher.h"
#include "gui.h"
#include "textures.h"
#include "texteditor.h"
#include "utils.h"

namespace GUI {
    static const int sel_dist = 20;
    static const int start_y = 52;
    static const int max_entries = 11;
    static int start = 0;
    static const char *empty_dir = "This is an empty directory";
    
    void DisplayFileBrowser(MenuItem &item) {
        G2D::FontSetStyle(1.f, WHITE, INTRAFONT_ALIGN_LEFT);
        float height = G2D::GetTextHeight();
        
        bool cwd_long = cfg.cwd.length() > 42;
        intraFontPrintf(fonts[FONT_DEFAULT], 40, 36, cwd_long ? "%.42s..." : "%s", cfg.cwd.c_str());
        G2D::DrawRect(40, 43, 400, 3, SELECTOR_COLOUR);
        
        if ((device == BROWSE_STATE_INTERNAL) || (device == BROWSE_STATE_EXTERNAL)) {
            float fill = (static_cast<float>(item.used_storage) / item.total_storage) * 400.f;
            G2D::DrawRect(40, 43, fill, 3, TITLE_COLOUR);
        }
        
        if (item.entries.empty()) {
            G2D::FontSetStyle(1.f, cfg.dark_theme? WHITE : BLACK, INTRAFONT_ALIGN_CENTER);
            G2D::DrawText(240, 136, empty_dir);
            return;
        }
        
        bool cwd_matches = (item.checked_cwd == cfg.cwd);
        int end = std::min(static_cast<int>(item.entries.size()), start + max_entries);
        
        for (int i = start; i < end; i++) {
            const char *filename = item.entries[i].d_name;
            int y_pos = start_y + (sel_dist * (i - start));

            if (i == item.selected) {
                G2D::DrawRect(0, y_pos, 480, sel_dist, SELECTOR_COLOUR);
            }
            
            if (item.checked[i] && cwd_matches) {
                G2D::DrawImageScale(icon_check[cfg.dark_theme], 0, y_pos, 18.f, 18.f);
            }
            else {
                G2D::DrawImageScale(icon_uncheck[cfg.dark_theme], 0, y_pos, 18.f, 18.f);
            }
            
            if (FIO_S_ISDIR(item.entries[i].d_stat.st_mode)) {
                G2D::DrawImageScale(icon_dir[cfg.dark_theme], 20, y_pos, 18.f, 18.f);
            }
            else {
                FileType file_type = FS::GetFileType(filename);
                G2D::DrawImageScale(file_icons[file_type], 20, y_pos, 18.f, 18.f);
            }
            
            G2D::FontSetStyle(1.f, cfg.dark_theme? WHITE : BLACK, INTRAFONT_ALIGN_LEFT);
            
            bool is_truncated = (filename[42] != '\0');
            float text_y = y_pos + 10 + ((sel_dist - height) / 2);
            intraFontPrintf(fonts[FONT_DEFAULT], 45, text_y, is_truncated? "%.42s..." : "%s", filename);
        }
    }

    void ControlFileBrowser(MenuItem &item, int &ctrl) {
        int size = (item.entries.size() - 1);
        Utils::SetBounds(item.selected, 0, size);

        if (ctrl & PSP_CTRL_UP) {
            item.selected--;

            if (item.selected < 0) {
                item.selected = size;
            }
            if (size < max_entries) {
                start = 0;
            }
            else if (start > item.selected) {
                start--;
            }
            else if ((item.selected == size) && (size > (max_entries - 1))) {
                start = size - (max_entries - 1);
            }
        }
        else if (ctrl & PSP_CTRL_DOWN) {
            item.selected++;

            if (item.selected > size) {
                item.selected = 0;
            }
            if ((item.selected > (start + (max_entries - 1))) && ((start + (max_entries - 1)) < size)) {
                start++;
            }
            if (item.selected == 0) {
                start = 0;
            }
        }

        if (Utils::IsButtonPressed(PSP_CTRL_LEFT)) {
            item.selected = 0;
            start = 0;
        }
        else if (Utils::IsButtonPressed(PSP_CTRL_RIGHT)) {
            item.selected = item.entries.size() - 1;

            if (size > max_entries) {
                start = size - (max_entries - 1);
            }
        }

        if (Utils::IsButtonPressed(PSP_CTRL_ENTER)) {
            if (FIO_S_ISDIR(item.entries[item.selected].d_stat.st_mode)) {
                if (item.entries.size() != 0) {
                    if (R_SUCCEEDED(FS::ChangeDirNext(item.entries[item.selected].d_name, item.entries))) {
                        start = 0;
                        // Make a copy before resizing our vector.
                        if ((item.checked_count > 1) && (item.checked_copy.empty())) {
                            item.checked_copy = item.checked;
                        }
                        
                        item.checked.resize(item.entries.size());
                        item.selected = 0;
                    }
                }
            }
            else {
                std::string path = FS::BuildPath(cfg.cwd, item.entries[item.selected].d_name);
                FileType file_type = FS::GetFileType(item.entries[item.selected].d_name);
                
                switch(file_type) {
                    case FileTypeApp:
                        GameLauncher::DisplayLauncher(path);
                        break;
                    
                    case FileTypeAudio:
                        AudioPlayer::Play(item);
                        break;

                    case FileTypeArchive:
                        if (R_SUCCEEDED(ArchiveHelper::Extract(path))) {
                            FS::GetDirList(cfg.cwd, item.entries);
                            GUI::ResetCheckbox(item);
                        }
                        break;
                    
                    case FileTypeImage:
                        item.texture = Textures::LoadImage(path.c_str(), item.entries[item.selected].d_stat.st_size);
                        if (item.texture) {
                            item.state = MENU_STATE_IMAGEVIEWER;
                        }
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
            if (R_SUCCEEDED(FS::ChangeDirPrev(item.entries))) {
                // Make a copy before resizing our vector.
                if (item.checked_count > 1) {
                    item.checked_copy = item.checked;
                }
                    
                item.checked.resize(item.entries.size());
                item.selected = 0;
                start = 0;
            }
        }
        else if (Utils::IsButtonPressed(PSP_CTRL_SQUARE)) {
            if ((!item.checked_cwd.empty()) && (item.checked_cwd.compare(cfg.cwd) != 0)) {
                GUI::ResetCheckbox(item);
            }
                
            item.checked_cwd = cfg.cwd;
            item.checked[item.selected] = !item.checked[item.selected];
            item.checked_count = std::count(item.checked.begin(), item.checked.end(), true);
        }
        else if (Utils::IsButtonPressed(PSP_CTRL_TRIANGLE)) {
            item.state = MENU_STATE_OPTIONS;
        }

        if (Utils::IsButtonPressed(PSP_CTRL_SELECT)) {
            item.state = MENU_STATE_MENUBAR;
        }
    }
}
