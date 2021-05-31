#include <pspctrl.h>

#include "config.h"
#include "fs.h"
#include "colours.h"
#include "g2d.h"
#include "gui.h"
#include "textures.h"
#include "utils.h"

static int row = 0, column = 0;
static bool copy = false, move = false, options_more = false;

namespace Options {
    static void ResetSelector(void) {
        row = 0;
        column = 0;
    }

    static void HandleMultipleCopy(MenuItem *item, int (*func)()) {
        int ret = 0;
        std::vector<SceIoDirent> entries;
        
        if (R_FAILED(ret = FS::GetDirList(item->checked_cwd.data(), entries)))
            return;
            
        for (u32 i = 0; i < item->checked_copy.size(); i++) {
            if (item->checked_copy.at(i)) {
                FS::Copy(&entries[i], item->checked_cwd);
                if (R_FAILED((*func)())) {
                    FS::GetDirList(cfg.cwd, item->entries);
                    GUI::ResetCheckbox(item);
                    break;
                }
            }
        }
        
        FS::GetDirList(cfg.cwd, item->entries);
        GUI::ResetCheckbox(item);
        entries.clear();
    }

    static void CreateFolder(MenuItem *item) {
        std::string name = G2D::KeyboardGetText("Enter folder name", "New folder");
        std::string path = FS::BuildPath(cfg.cwd, name);
        
        if (R_SUCCEEDED(FS::MakeDir(path.c_str()))) {
            FS::GetDirList(cfg.cwd, item->entries);
            GUI::ResetCheckbox(item);
        }
    }

    static void CreateFile(MenuItem *item) {
        std::string name = G2D::KeyboardGetText("Enter file name", "New File");
        std::string path = FS::BuildPath(cfg.cwd, name);
        
        if (R_SUCCEEDED(FS::CreateFile(path.c_str()))) {
            FS::GetDirList(cfg.cwd, item->entries);
            GUI::ResetCheckbox(item);
        }
    }

    static void Rename(MenuItem *item, const std::string &filename) {
        std::string src_path = FS::BuildPath(cfg.cwd, item->entries[item->selected].d_name);
        std::string name = G2D::KeyboardGetText("Enter new name", filename);
        std::string dest_path = FS::BuildPath(cfg.cwd, name);

        if (R_SUCCEEDED(sceIoRename(src_path.c_str(), dest_path.c_str()))) {
            FS::GetDirList(cfg.cwd, item->entries);
            Options::ResetSelector();
            options_more = false;
            item->state = MENU_STATE_FILEBROWSER;
        }
    }

    static void Copy(MenuItem *item) {
        if (!copy) {
            if ((item->checked_count >= 1) && (item->checked_cwd.compare(cfg.cwd) != 0))
                GUI::ResetCheckbox(item);
            if (item->checked_count <= 1)
                FS::Copy(&item->entries[item->selected], cfg.cwd);
            
            copy = !copy;
            item->state = MENU_STATE_FILEBROWSER;
        }
        else {
            if ((item->checked_count > 1) && (item->checked_cwd.compare(cfg.cwd) != 0))
                Options::HandleMultipleCopy(item, &FS::Paste);
            else {
                if (R_SUCCEEDED(FS::Paste())) {
                    FS::GetDirList(cfg.cwd, item->entries);
                    GUI::ResetCheckbox(item);
                }
            }
            
            GUI::GetStorageSize(item);
            copy = !copy;
            item->state = MENU_STATE_FILEBROWSER;
        }
    }

    static void Move(MenuItem *item) {
        if (!move) {
            if ((item->checked_count >= 1) && (item->checked_cwd.compare(cfg.cwd) != 0))
                GUI::ResetCheckbox(item);
                
            if (item->checked_count <= 1)
                FS::Copy(&item->entries[item->selected], cfg.cwd);
        }
        else {
            if ((item->checked_count > 1) && (item->checked_cwd.compare(cfg.cwd) != 0))
                Options::HandleMultipleCopy(item, &FS::Move);
            else if (R_SUCCEEDED(FS::Move())) {
                FS::GetDirList(cfg.cwd, item->entries);
                GUI::ResetCheckbox(item);
            }
        }
        
        move = !move;
        item->state = MENU_STATE_FILEBROWSER;
    }
}

namespace GUI {
    void DisplayFileOptions(MenuItem *item) {
        G2D::DrawRect(0, 18, 480, 254, G2D_RGBA(0, 0, 0, cfg.dark_theme? 50: 80));
        G2D::DrawImage(cfg.dark_theme? options_dialog_dark : options_dialog, (480 - options_dialog->w) / 2, (272 - options_dialog->h) / 2);
        G2D::FontSetStyle(font, 1.0f, TITLE_COLOUR, INTRAFONT_ALIGN_LEFT);
        G2D::DrawText(140, 52, "Actions");
        
        if (row == 0 && column == 0)
            G2D::DrawRect(132, 71, 107, 38, SELECTOR_COLOUR);
        else if (row == 1 && column == 0)
            G2D::DrawRect(241, 71, 107, 38, SELECTOR_COLOUR);
        else if (row == 0 && column == 1)
            G2D::DrawRect(132, 110, 107, 38, SELECTOR_COLOUR);
        else if (row == 1 && column == 1)
            G2D::DrawRect(241, 110, 107, 38, SELECTOR_COLOUR);
        else if (row == 0 && column == 2 && !options_more)
            G2D::DrawRect(132, 148, 107, 38, SELECTOR_COLOUR);
        else if (row == 1 && column == 2 && !options_more)
            G2D::DrawRect(241, 148, 107, 38, SELECTOR_COLOUR);
        else if (column == 3 && !options_more)
            G2D::DrawRect((340 - intraFontMeasureText(font, "CANCEL")) - 5, (230 - (font->texYSize - 6)) - 5, intraFontMeasureText(font, "CANCEL") + 10,
                (font->texYSize - 6) + 10, SELECTOR_COLOUR);
        else if (column == 2 && options_more)
            G2D::DrawRect((340 - intraFontMeasureText(font, "CANCEL")) - 5, (230 - (font->texYSize - 6)) - 5, intraFontMeasureText(font, "CANCEL") + 10, 
                (font->texYSize - 6) + 10, SELECTOR_COLOUR);
                
        G2D::DrawText(340 - intraFontMeasureText(font, "CANCEL"), 230 - (font->texYSize - 15), "CANCEL");
        G2D::FontSetStyle(font, 1.0f, TEXT_COLOUR, INTRAFONT_ALIGN_LEFT);
        
        if (!options_more) {
            G2D::DrawText(143, 95, "Properties");
            G2D::DrawText(143, 133, copy? "Paste" : "Copy");
            G2D::DrawText(143, 171, "Delete");
            G2D::DrawText(247, 95, "Refresh");
            G2D::DrawText(247, 133, move? "Paste" : "Move");
            G2D::DrawText(247, 171, "More...");
        }
        else {
            G2D::DrawText(143, 95, "New folder");
            G2D::DrawText(143, 133, "Rename");
            G2D::DrawText(247, 95, "New file");
        }
    }

    void ControlFileOptions(MenuItem *item, int *ctrl) {
        if (*ctrl & PSP_CTRL_RIGHT)
            row++;
        else if (*ctrl & PSP_CTRL_LEFT)
            row--;
            
        if (*ctrl & PSP_CTRL_DOWN)
            column++;
        else if (*ctrl & PSP_CTRL_UP)
            column--;
        
        if (!options_more) {
            Utils::SetBounds(&row, 0, 1);
            Utils::SetBounds(&column, 0, 3);
        }
        else {
            Utils::SetBounds(&column, 0, 2);
            
            if (column == 1)
                Utils::SetBounds(&row, 0, 0);
            else
                Utils::SetBounds(&row, 0, 1);
        }

        if (Utils::IsButtonPressed(PSP_CTRL_CROSS)) {
            const std::string filename = item->entries[item->selected].d_name;

            if (row == 0) {
                if (!options_more) {
                    if (column == 0)
                        item->state = MENU_STATE_PROPERTIES;
                    else if (column == 1)
                        Options::Copy(item);
                    else if (column == 2)
                        item->state = MENU_STATE_DELETE;
                }
                else {
                    if (column == 0)
                        Options::CreateFolder(item);
                    else if (column == 1)
                        Options::Rename(item, filename);
                }
            }
            else if (row == 1) {
                if (!options_more) {
                    if (column == 0) {
                        FS::GetDirList(cfg.cwd, item->entries);
                        Options::ResetSelector();
                        options_more = false;
                        item->selected = 0;
                        item->state = MENU_STATE_FILEBROWSER;
                    }
                    else if (column == 1)
                        Options::Move(item);
                    else if (column == 2) {
                        Options::ResetSelector();
                        options_more = true;
                    }
                }
                else {
                    if (column == 0)
                        Options::CreateFile(item);
                }
            }
            if (column == 3) {
                copy = false;
                move = false;
                Options::ResetSelector();
                options_more = false;
                item->state = MENU_STATE_FILEBROWSER;
            }
        }
        if (Utils::IsButtonPressed(PSP_CTRL_CANCEL)) {
            Options::ResetSelector();

            if (!options_more)
                item->state = MENU_STATE_FILEBROWSER;
            else
                options_more = false;
        }
    }
}
