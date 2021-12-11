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
    static float pos_x = -180.f;
    static const float pos_x_bounds = 0.f;

    void HandleMenubarAnim(float *delta_time) {
        pos_x += 1000 * (*delta_time);
        
        if (pos_x > 0)
            pos_x = pos_x_bounds;
    }

    void DisplayMenubar(void) {
        G2D::DrawImage(bg_header, pos_x, 18);
        G2D::DrawRect(pos_x, 90, 180, 254, BG_COLOUR);
        G2D::DrawRect(pos_x + 180, 18, 480, 254, G2D_RGBA(0, 0, 0, cfg.dark_theme? 50: 80));
        G2D::DrawRect(pos_x, 90 + (30 * selection), 180, 30, SELECTOR_COLOUR);

        G2D::FontSetStyle(font, 1.0f, cfg.dark_theme? WHITE : BLACK, INTRAFONT_ALIGN_LEFT);
        
        if (is_psp_go) {
            G2D::DrawImage(icon_sd[cfg.dark_theme], pos_x + 10, 92);
            G2D::DrawText(pos_x + 50, 90 + ((30 - (font->glyph->height - 6)) / 2), !is_ms_inserted? "ef0:/" : "ms0:/");
            
            if (is_ms_inserted) {
                G2D::DrawImage(icon_secure[cfg.dark_theme], pos_x + 10, 122);
                G2D::DrawText(pos_x + 50, 90 + ((30 - (font->glyph->height - 6)) / 2) + 30, "ef0:/");
                
                if (cfg.dev_options) {
                    G2D::DrawImage(icon_secure[cfg.dark_theme], pos_x + 10, 152);
                    G2D::DrawText(pos_x + 50, 90 + ((30 - (font->glyph->height - 6)) / 2) + 60, "flash0:/");
                    
                    G2D::DrawImage(icon_secure[cfg.dark_theme], pos_x + 10, 182);
                    G2D::DrawText(pos_x + 50, 90 + ((30 - (font->glyph->height - 6)) / 2) + 90, "flash1:/");
                    
                    G2D::DrawImage(icon_secure[cfg.dark_theme], pos_x + 10, 212);
                    G2D::DrawText(pos_x + 50, 90 + ((30 - (font->glyph->height - 6)) / 2) + 120, "flash2:/");
                    
                    G2D::DrawImage(icon_secure[cfg.dark_theme], pos_x + 10, 242);
                    G2D::DrawText(pos_x + 50, 90 + ((30 - (font->glyph->height - 6)) / 2) + 150, "flash3:/");
                }
            }
            else {
                if (cfg.dev_options) {
                    G2D::DrawImage(icon_secure[cfg.dark_theme], pos_x + 10, 122);
                    G2D::DrawText(pos_x + 50, 90 + ((30 - (font->glyph->height - 6)) / 2) + 30, "flash0:/");
                    
                    G2D::DrawImage(icon_secure[cfg.dark_theme], pos_x + 10, 152);
                    G2D::DrawText(pos_x + 50, 90 + ((30 - (font->glyph->height - 6)) / 2) + 60, "flash1:/");
                    
                    G2D::DrawImage(icon_secure[cfg.dark_theme], pos_x + 10, 182);
                    G2D::DrawText(pos_x + 50, 90 + ((30 - (font->glyph->height - 6)) / 2) + 90, "flash2:/");
                    
                    G2D::DrawImage(icon_secure[cfg.dark_theme], pos_x + 10, 212);
                    G2D::DrawText(pos_x + 50, 90 + ((30 - (font->glyph->height - 6)) / 2) + 120, "flash3:/");
                }
            }
        }
        else {
            G2D::DrawImage(icon_sd[cfg.dark_theme], pos_x + 10, 92);
            G2D::DrawText(pos_x + 50, 90 + ((30 - (font->glyph->height - 6)) / 2), "ms0:/");
            
            if (cfg.dev_options) {
                G2D::DrawImage(icon_secure[cfg.dark_theme], pos_x + 10, 122);
                G2D::DrawText(pos_x + 50, 90 + ((30 - (font->glyph->height - 6)) / 2) + 30, "flash0:/");
                
                G2D::DrawImage(icon_secure[cfg.dark_theme], pos_x + 10, 152);
                G2D::DrawText(pos_x + 50, 90 + ((30 - (font->glyph->height - 6)) / 2) + 60, "flash1:/");
                
                G2D::DrawImage(icon_secure[cfg.dark_theme], pos_x + 10, 182);
                G2D::DrawText(pos_x + 50, 90 + ((30 - (font->glyph->height - 6)) / 2) + 90, "flash2:/");
                
                G2D::DrawImage(icon_secure[cfg.dark_theme], pos_x + 10, 212);
                G2D::DrawText(pos_x + 50, 90 + ((30 - (font->glyph->height - 6)) / 2) + 120, "flash3:/");
                
                G2D::DrawImage(icon_secure[cfg.dark_theme], pos_x + 10, 242);
                G2D::DrawText(pos_x + 50, 90 + ((30 - (font->glyph->height - 6)) / 2) + 150, "disc0:/");
            }
        }
    }

    void ControlMenubar(MenuItem *item, int *ctrl) {
        if (*ctrl & PSP_CTRL_UP)
            selection--;
        else if (*ctrl & PSP_CTRL_DOWN)
            selection++;
            
        if (is_psp_go) {
            if (cfg.dev_options) {
                Utils::SetMax(selection, 0, is_ms_inserted? 5 : 4);
                Utils::SetMin(selection, is_ms_inserted? 5 : 4, 0);
            }
            else {
                Utils::SetMax(selection, 0, is_ms_inserted? 1 : 0);
                Utils::SetMin(selection, is_ms_inserted? 1 : 0, 0);
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
                    if ((is_psp_go && is_ms_inserted) || (!is_psp_go)) {
                        cfg.cwd = "ms0:";
                        device = BROWSE_STATE_EXTERNAL;
                    }
                    else if (is_psp_go && !is_ms_inserted) {
                        cfg.cwd = "ef0:";
                        device = BROWSE_STATE_INTERNAL;
                    }
                    break;

                case 1:
                    if (!(is_psp_go && is_ms_inserted)) {
                        if ((R_FAILED(ret = sceIoUnassign("flash0:"))) && (ret != 0x80020321))
                            Log::Error("sceIoUnassign(flash0) failed: 0x%x\n", ret);
                            
                        if (R_FAILED(ret = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, nullptr, 0)))
                            Log::Error("sceIoAssign(flash0) failed: 0x%x\n", ret);
                    }

                    cfg.cwd = (is_psp_go && is_ms_inserted)? "ef0:" : "flash0:/";
                    device = (is_psp_go && is_ms_inserted)? BROWSE_STATE_INTERNAL : BROWSE_STATE_FLASH0;
                    break;

                case 2:
                    if (is_psp_go && is_ms_inserted) {
                        if ((R_FAILED(ret = sceIoUnassign("flash0:"))) && (ret != 0x80020321))
                            Log::Error("sceIoUnassign(flash0) failed: 0x%x\n", ret);
                            
                        if (R_FAILED(ret = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, nullptr, 0)))
                            Log::Error("sceIoAssign(flash0) failed: 0x%x\n", ret);
                    }
                    else {
                        if ((R_FAILED(ret = sceIoUnassign("flash1:"))) && (ret != 0x80020321))
                            Log::Error("sceIoUnassign(flash1) failed: 0x%x\n", ret);
                            
                        if (R_FAILED(ret = sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, nullptr, 0)))
                            Log::Error("sceIoAssign(flash1) failed: 0x%x\n", ret);
                    }

                    cfg.cwd = (is_psp_go && is_ms_inserted)? "flash0:/" : "flash1:/";
                    device = (is_psp_go && is_ms_inserted)? BROWSE_STATE_FLASH0 : BROWSE_STATE_FLASH1;
                    break;

                case 3:
                    if (is_psp_go && is_ms_inserted) {
                        if ((R_FAILED(ret = sceIoUnassign("flash1:"))) && (ret != 0x80020321))
                            Log::Error("sceIoUnassign(flash1) failed: 0x%x\n", ret);
                            
                        if (R_FAILED(ret = sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, nullptr, 0)))
                            Log::Error("sceIoAssign(flash1) failed: 0x%x\n", ret);
                    }
                    else {
                        if ((R_FAILED(ret = sceIoUnassign("flash2:"))) && (ret != 0x80020321))
                            Log::Error("sceIoUnassign(flash2) failed: 0x%x\n", ret);
                            
                        if (R_FAILED(ret = sceIoAssign("flash2:", "lflash0:0,2", "flashfat2:", IOASSIGN_RDWR, nullptr, 0)))
                            Log::Error("sceIoAssign(flash2) failed: 0x%x\n", ret);
                    }

                    cfg.cwd = (is_psp_go && is_ms_inserted)? "flash1:/" : "flash2:/";
                    device = (is_psp_go && is_ms_inserted)? BROWSE_STATE_FLASH1 : BROWSE_STATE_FLASH2;
                    break;

                case 4:
                    if (is_psp_go && is_ms_inserted) {
                        if ((R_FAILED(ret = sceIoUnassign("flash2:"))) && (ret != 0x80020321))
                            Log::Error("sceIoUnassign(flash2) failed: 0x%x\n", ret);
                            
                        if (R_FAILED(ret = sceIoAssign("flash2:", "lflash0:0,2", "flashfat2:", IOASSIGN_RDWR, nullptr, 0)))
                            Log::Error("sceIoAssign(flash2) failed: 0x%x\n", ret);
                    }
                    else {
                        if ((R_FAILED(ret = sceIoUnassign("flash3:"))) && (ret != 0x80020321))
                            Log::Error("sceIoUnassign(flash3) failed: 0x%x\n", ret);
                            
                        if (R_FAILED(ret = sceIoAssign("flash3:", "lflash0:0,3", "flashfat3:", IOASSIGN_RDWR, nullptr, 0)))
                            Log::Error("sceIoAssign(flash3) failed: 0x%x\n", ret);
                    }

                    cfg.cwd = (is_psp_go && is_ms_inserted)? "flash2:/" : "flash3:/";
                    device = (is_psp_go && is_ms_inserted)? BROWSE_STATE_FLASH2 : BROWSE_STATE_FLASH3;
                    break;

                case 5:
                    if (is_psp_go && is_ms_inserted) {
                        if ((R_FAILED(ret = sceIoUnassign("flash3:"))) && (ret != 0x80020321))
                            Log::Error("sceIoUnassign(flash3) failed: 0x%x\n", ret);
                        
                        if (R_FAILED(ret = sceIoAssign("flash3:", "lflash0:0,3", "flashfat3:", IOASSIGN_RDWR, nullptr, 0)))
                            Log::Error("sceIoAssign(flash3) failed: 0x%x\n", ret);

                        cfg.cwd = "flash3:/";
                        device = BROWSE_STATE_FLASH3;
                    }
                    else if (!is_psp_go) {
                        if (sceUmdCheckMedium() != 0) {
                            if (R_FAILED(ret = sceUmdActivate(1, "disc0:")))
                                Log::Error("sceUmdActivate(disc0) failed: 0x%x\n", ret);
                            
                            if (R_FAILED(ret = sceUmdWaitDriveStat(PSP_UMD_READY)))
                                Log::Error("sceUmdWaitDriveStat() failed: 0x%x\n", ret);
                        }
                        
                        cfg.cwd = "disc0:/";
                        device = BROWSE_STATE_UMD;
                    }
                    break;
            }
            
            pos_x -= 10.0;
            pos_x = -180;
            FS::GetDirList(cfg.cwd, item->entries);
            if ((device == BROWSE_STATE_FLASH0) || (device == BROWSE_STATE_FLASH1) || (device == BROWSE_STATE_FLASH2) || (device == BROWSE_STATE_FLASH3))
                cfg.cwd.pop_back();

            item->state = MENU_STATE_FILEBROWSER;
        }
        else if ((Utils::IsButtonPressed(PSP_CTRL_CANCEL)) || (Utils::IsButtonPressed(PSP_CTRL_SELECT))) {
            pos_x -= 10.0;
            pos_x = -180;
            item->state = MENU_STATE_FILEBROWSER;
        }
    }
}
