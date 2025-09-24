#include <pspumd.h>

#include "colours.h"
#include "config.h"
#include "fs.h"
#include "g2d.h"
#include "gui.h"
#include "log.h"
#include "textures.h"
#include "utils.h"

namespace GUI {
    static int selection = 0;
    static float posX = -180.f;
    static const float posXBounds = 0.f;

    void HandleMenubarAnim(float &delta) {
        posX += delta;
        
        if (posX > 0) {
            posX = posXBounds;
        }
    }

    void DisplayMenubar(void) {
        G2D::DrawImage(bg_header, posX, 18);
        G2D::DrawRect(posX, 90, 180, 254, BG_COLOUR);
        G2D::DrawRect(posX + 180, 18, 480, 254, G2D_RGBA(0, 0, 0, cfg.dark_theme? 50 : 80));
        G2D::DrawRect(posX, 90 + (30 * selection), 180, 30, SELECTOR_COLOUR);

        G2D::FontSetStyle(1.f, cfg.dark_theme? WHITE : BLACK, INTRAFONT_ALIGN_LEFT);
        
        if (isPSPGo) {
            G2D::DrawImage(icon_sd[cfg.dark_theme], posX + 10, 92);
            G2D::DrawText(posX + 50, 90 + ((30 - (fonts[FONT_DEFAULT]->glyph->height - 6)) / 2), !isMSInserted? "ef0:/" : "ms0:/");
            
            if (isMSInserted) {
                G2D::DrawImage(icon_secure[cfg.dark_theme], posX + 10, 122);
                G2D::DrawText(posX + 50, 90 + ((30 - (fonts[FONT_DEFAULT]->glyph->height - 6)) / 2) + 30, "ef0:/");
                
                if (cfg.dev_options) {
                    G2D::DrawImage(icon_secure[cfg.dark_theme], posX + 10, 152);
                    G2D::DrawText(posX + 50, 90 + ((30 - (fonts[FONT_DEFAULT]->glyph->height - 6)) / 2) + 60, "flash0:/");
                    
                    G2D::DrawImage(icon_secure[cfg.dark_theme], posX + 10, 182);
                    G2D::DrawText(posX + 50, 90 + ((30 - (fonts[FONT_DEFAULT]->glyph->height - 6)) / 2) + 90, "flash1:/");
                    
                    G2D::DrawImage(icon_secure[cfg.dark_theme], posX + 10, 212);
                    G2D::DrawText(posX + 50, 90 + ((30 - (fonts[FONT_DEFAULT]->glyph->height - 6)) / 2) + 120, "flash2:/");
                    
                    G2D::DrawImage(icon_secure[cfg.dark_theme], posX + 10, 242);
                    G2D::DrawText(posX + 50, 90 + ((30 - (fonts[FONT_DEFAULT]->glyph->height - 6)) / 2) + 150, "flash3:/");
                }
            }
            else {
                if (cfg.dev_options) {
                    G2D::DrawImage(icon_secure[cfg.dark_theme], posX + 10, 122);
                    G2D::DrawText(posX + 50, 90 + ((30 - (fonts[FONT_DEFAULT]->glyph->height - 6)) / 2) + 30, "flash0:/");
                    
                    G2D::DrawImage(icon_secure[cfg.dark_theme], posX + 10, 152);
                    G2D::DrawText(posX + 50, 90 + ((30 - (fonts[FONT_DEFAULT]->glyph->height - 6)) / 2) + 60, "flash1:/");
                    
                    G2D::DrawImage(icon_secure[cfg.dark_theme], posX + 10, 182);
                    G2D::DrawText(posX + 50, 90 + ((30 - (fonts[FONT_DEFAULT]->glyph->height - 6)) / 2) + 90, "flash2:/");
                    
                    G2D::DrawImage(icon_secure[cfg.dark_theme], posX + 10, 212);
                    G2D::DrawText(posX + 50, 90 + ((30 - (fonts[FONT_DEFAULT]->glyph->height - 6)) / 2) + 120, "flash3:/");
                }
            }
        }
        else {
            G2D::DrawImage(icon_sd[cfg.dark_theme], posX + 10, 92);
            G2D::DrawText(posX + 50, 90 + ((30 - (fonts[FONT_DEFAULT]->glyph->height - 6)) / 2), "ms0:/");
            
            if (cfg.dev_options) {
                G2D::DrawImage(icon_secure[cfg.dark_theme], posX + 10, 122);
                G2D::DrawText(posX + 50, 90 + ((30 - (fonts[FONT_DEFAULT]->glyph->height - 6)) / 2) + 30, "flash0:/");
                
                G2D::DrawImage(icon_secure[cfg.dark_theme], posX + 10, 152);
                G2D::DrawText(posX + 50, 90 + ((30 - (fonts[FONT_DEFAULT]->glyph->height - 6)) / 2) + 60, "flash1:/");
                
                G2D::DrawImage(icon_secure[cfg.dark_theme], posX + 10, 182);
                G2D::DrawText(posX + 50, 90 + ((30 - (fonts[FONT_DEFAULT]->glyph->height - 6)) / 2) + 90, "flash2:/");
                
                G2D::DrawImage(icon_secure[cfg.dark_theme], posX + 10, 212);
                G2D::DrawText(posX + 50, 90 + ((30 - (fonts[FONT_DEFAULT]->glyph->height - 6)) / 2) + 120, "flash3:/");
                
                G2D::DrawImage(icon_secure[cfg.dark_theme], posX + 10, 242);
                G2D::DrawText(posX + 50, 90 + ((30 - (fonts[FONT_DEFAULT]->glyph->height - 6)) / 2) + 150, "disc0:/");
            }
        }
    }

    void ControlMenubar(MenuItem &item, int &ctrl) {
        if (ctrl & PSP_CTRL_UP) {
            selection--;
        }
        else if (ctrl & PSP_CTRL_DOWN) {
            selection++;
        }
            
        if (isPSPGo) {
            if (cfg.dev_options) {
                Utils::SetMax(selection, 0, isMSInserted? 5 : 4);
                Utils::SetMin(selection, isMSInserted? 5 : 4, 0);
            }
            else {
                Utils::SetMax(selection, 0, isMSInserted? 1 : 0);
                Utils::SetMin(selection, isMSInserted? 1 : 0, 0);
            }
        }
        else {
            Utils::SetMax(selection, 0, cfg.dev_options? 5 : 0);
            Utils::SetMin(selection, cfg.dev_options? 5 : 0, 0);
        }
        
        if (Utils::IsButtonPressed(PSP_CTRL_ENTER)) {
            unsigned int ret = 0;

            switch (selection) {
                case 0:
                    if ((isPSPGo && isMSInserted) || (!isPSPGo)) {
                        cfg.cwd = "ms0:";
                        device = BROWSE_STATE_EXTERNAL;
                    }
                    else if (isPSPGo && !isMSInserted) {
                        cfg.cwd = "ef0:";
                        device = BROWSE_STATE_INTERNAL;
                    }
                    break;

                case 1:
                    if (!(isPSPGo && isMSInserted)) {
                        if ((R_FAILED(ret = sceIoUnassign("flash0:"))) && (ret != 0x80020321)) {
                            Log::Error("sceIoUnassign(flash0) failed: 0x%x\n", ret);
                        }
                            
                        if (R_FAILED(ret = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, nullptr, 0))) {
                            Log::Error("sceIoAssign(flash0) failed: 0x%x\n", ret);
                        }
                    }

                    cfg.cwd = (isPSPGo && isMSInserted)? "ef0:" : "flash0:/";
                    device = (isPSPGo && isMSInserted)? BROWSE_STATE_INTERNAL : BROWSE_STATE_FLASH0;
                    break;

                case 2:
                    if (isPSPGo && isMSInserted) {
                        if ((R_FAILED(ret = sceIoUnassign("flash0:"))) && (ret != 0x80020321)) {
                            Log::Error("sceIoUnassign(flash0) failed: 0x%x\n", ret);
                        }
                            
                        if (R_FAILED(ret = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, nullptr, 0))) {
                            Log::Error("sceIoAssign(flash0) failed: 0x%x\n", ret);
                        }
                    }
                    else {
                        if ((R_FAILED(ret = sceIoUnassign("flash1:"))) && (ret != 0x80020321)) {
                            Log::Error("sceIoUnassign(flash1) failed: 0x%x\n", ret);
                        }
                            
                        if (R_FAILED(ret = sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, nullptr, 0))) {
                            Log::Error("sceIoAssign(flash1) failed: 0x%x\n", ret);
                        }
                    }

                    cfg.cwd = (isPSPGo && isMSInserted)? "flash0:/" : "flash1:/";
                    device = (isPSPGo && isMSInserted)? BROWSE_STATE_FLASH0 : BROWSE_STATE_FLASH1;
                    break;

                case 3:
                    if (isPSPGo && isMSInserted) {
                        if ((R_FAILED(ret = sceIoUnassign("flash1:"))) && (ret != 0x80020321)) {
                            Log::Error("sceIoUnassign(flash1) failed: 0x%x\n", ret);
                        }
                            
                        if (R_FAILED(ret = sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, nullptr, 0))) {
                            Log::Error("sceIoAssign(flash1) failed: 0x%x\n", ret);
                        }
                    }
                    else {
                        if ((R_FAILED(ret = sceIoUnassign("flash2:"))) && (ret != 0x80020321)) {
                            Log::Error("sceIoUnassign(flash2) failed: 0x%x\n", ret);
                        }
                            
                        if (R_FAILED(ret = sceIoAssign("flash2:", "lflash0:0,2", "flashfat2:", IOASSIGN_RDWR, nullptr, 0))) {
                            Log::Error("sceIoAssign(flash2) failed: 0x%x\n", ret);
                        }
                    }

                    cfg.cwd = (isPSPGo && isMSInserted)? "flash1:/" : "flash2:/";
                    device = (isPSPGo && isMSInserted)? BROWSE_STATE_FLASH1 : BROWSE_STATE_FLASH2;
                    break;

                case 4:
                    if (isPSPGo && isMSInserted) {
                        if ((R_FAILED(ret = sceIoUnassign("flash2:"))) && (ret != 0x80020321)) {
                            Log::Error("sceIoUnassign(flash2) failed: 0x%x\n", ret);
                        }
                            
                        if (R_FAILED(ret = sceIoAssign("flash2:", "lflash0:0,2", "flashfat2:", IOASSIGN_RDWR, nullptr, 0))) {
                            Log::Error("sceIoAssign(flash2) failed: 0x%x\n", ret);
                        }
                    }
                    else {
                        if ((R_FAILED(ret = sceIoUnassign("flash3:"))) && (ret != 0x80020321)) {
                            Log::Error("sceIoUnassign(flash3) failed: 0x%x\n", ret);
                        }
                            
                        if (R_FAILED(ret = sceIoAssign("flash3:", "lflash0:0,3", "flashfat3:", IOASSIGN_RDWR, nullptr, 0))) {
                            Log::Error("sceIoAssign(flash3) failed: 0x%x\n", ret);
                        }
                    }

                    cfg.cwd = (isPSPGo && isMSInserted)? "flash2:/" : "flash3:/";
                    device = (isPSPGo && isMSInserted)? BROWSE_STATE_FLASH2 : BROWSE_STATE_FLASH3;
                    break;

                case 5:
                    if (isPSPGo && isMSInserted) {
                        if ((R_FAILED(ret = sceIoUnassign("flash3:"))) && (ret != 0x80020321)) {
                            Log::Error("sceIoUnassign(flash3) failed: 0x%x\n", ret);
                        }
                        
                        if (R_FAILED(ret = sceIoAssign("flash3:", "lflash0:0,3", "flashfat3:", IOASSIGN_RDWR, nullptr, 0))) {
                            Log::Error("sceIoAssign(flash3) failed: 0x%x\n", ret);
                        }

                        cfg.cwd = "flash3:/";
                        device = BROWSE_STATE_FLASH3;
                    }
                    else if (!isPSPGo) {
                        if (sceUmdCheckMedium() != 0) {
                            if (R_FAILED(ret = sceUmdActivate(1, "disc0:"))) {
                                Log::Error("sceUmdActivate(disc0) failed: 0x%x\n", ret);
                            }
                            
                            if (R_FAILED(ret = sceUmdWaitDriveStat(PSP_UMD_READY))) {
                                Log::Error("sceUmdWaitDriveStat() failed: 0x%x\n", ret);
                            }
                        }
                        
                        cfg.cwd = "disc0:/";
                        device = BROWSE_STATE_UMD;
                    }
                    break;
            }
            
            posX -= 10.0;
            posX = -180;
            FS::GetDirList(cfg.cwd, item.entries);
            if ((device == BROWSE_STATE_FLASH0) || (device == BROWSE_STATE_FLASH1) || (device == BROWSE_STATE_FLASH2) || (device == BROWSE_STATE_FLASH3)) {
                cfg.cwd.pop_back();
            }

            item.state = MENU_STATE_FILEBROWSER;
        }
        else if ((Utils::IsButtonPressed(PSP_CTRL_CANCEL)) || (Utils::IsButtonPressed(PSP_CTRL_SELECT))) {
            posX -= 10.0;
            posX = -180;
            item.state = MENU_STATE_FILEBROWSER;
        }
    }
}
