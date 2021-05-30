#include <cstring>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspkernel.h>
#include <pspnet.h>
#include <pspnet_apctl.h>
#include <pspnet_inet.h>
#include <psppower.h>
#include <psputility.h>

#include "config.h"
#include "colours.h"
#include "fs.h"
#include "ftppsp.h"
#include "g2d.h"
#include "log.h"
#include "gui.h"
#include "textures.h"
#include "utils.h"

namespace FTP {
    static bool running = 1;
    
    static int DisplayNetDialog(void) {
        running = 1;
        bool done = 0;
        
        pspUtilityNetconfData data;
        std::memset(&data, 0, sizeof(data));
        
        data.base.size = sizeof(data);
        data.base.language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
        data.base.buttonSwap = PSP_UTILITY_ACCEPT_CROSS;
        data.base.graphicsThread = 17;
        data.base.accessThread = 19;
        data.base.fontThread = 18;
        data.base.soundThread = 16;
        data.action = PSP_NETCONF_ACTION_CONNECTAP;
        
        struct pspUtilityNetconfAdhoc adhocparam;
        std::memset(&adhocparam, 0, sizeof(adhocparam));
        
        data.adhocparam = &adhocparam;
        sceUtilityNetconfInitStart(&data);
        
        while(running) {
            g2dClear(BLACK_BG);
            g2dFlip(G2D_VSYNC);
            
            switch(sceUtilityNetconfGetStatus()) {
                case PSP_UTILITY_DIALOG_NONE:
                    running = 1;
                    break;
                    
                case PSP_UTILITY_DIALOG_VISIBLE:
                    sceUtilityNetconfUpdate(1);
                    running = 1;
                    break;
                
                case PSP_UTILITY_DIALOG_QUIT:
                    sceUtilityNetconfShutdownStart();
                    //running = 0;
                    break;
                    
                case PSP_UTILITY_DIALOG_FINISHED:
                    done = 1;
                    break;
                    
                default:
                    break;
            }
            
            if (done) {
                running = true;
                break;
            }
        }
        
        return 1;
    }
    
    static int InitNet(void) {
        int ret = 0;
        
        if (R_FAILED(ret = sceNetInit(128 * 1024, 42, 4 * 1024, 42, 4 * 1024)))
            return ret;
            
        if (R_FAILED(ret = sceNetInetInit()))
            return ret;
            
        if (R_FAILED(ret = sceNetApctlInit(0x8000, 48)))
            return ret;
            
        return 0;
    }
    
    static void ExitNet(void) {
        sceNetApctlTerm();
        sceNetInetTerm();
        sceNetTerm();
    }
    
    static void InitFlash(void) {
        unsigned int ret = 0;
        
        if ((R_FAILED(ret = sceIoUnassign("flash0:"))) && (ret != 0x80020321))
            Log::Error("sceIoUnassign(flash0) failed: 0x%x\n", ret);
            
        if (R_FAILED(ret = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, nullptr, 0)))
            Log::Error("sceIoAssign(flash0) failed: 0x%x\n", ret);
            
        if ((R_FAILED(ret = sceIoUnassign("flash1:"))) && (ret != 0x80020321))
            Log::Error("sceIoUnassign(flash1) failed: 0x%x\n", ret);
            
        if (R_FAILED(ret = sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, nullptr, 0)))
            Log::Error("sceIoAssign(flash1) failed: 0x%x\n", ret);
            
        if ((R_FAILED(ret = sceIoUnassign("flash2:"))) && (ret != 0x80020321))
            Log::Error("sceIoUnassign(flash2) failed: 0x%x\n", ret);
            
        if (R_FAILED(ret = sceIoAssign("flash2:", "lflash0:0,2", "flashfat2:", IOASSIGN_RDWR, nullptr, 0)))
            Log::Error("sceIoAssign(flash2) failed: 0x%x\n", ret);
            
        if ((R_FAILED(ret = sceIoUnassign("flash3:"))) && (ret != 0x80020321))
            Log::Error("sceIoUnassign(flash3) failed: 0x%x\n", ret);
        
        if (R_FAILED(ret = sceIoAssign("flash3:", "lflash0:0,3", "flashfat3:", IOASSIGN_RDWR, nullptr, 0)))
            Log::Error("sceIoAssign(flash3) failed: 0x%x\n", ret);
    }
    
    static void ExitFlash(void) {
        unsigned int ret = 0;
        
        if ((R_FAILED(ret = sceIoUnassign("flash0:"))) && (ret != 0x80020321))
            Log::Error("sceIoUnassign(flash0) failed: 0x%x\n", ret);
        
        if (R_FAILED(ret = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDONLY, nullptr, 0)))
            Log::Error("sceIoAssign(flash0) failed: 0x%x\n", ret);
            
        if ((R_FAILED(ret = sceIoUnassign("flash1:"))) && (ret != 0x80020321))
            Log::Error("sceIoUnassign(flash1) failed: 0x%x\n", ret);
            
        if (R_FAILED(ret = sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDONLY, nullptr, 0)))
            Log::Error("sceIoAssign(flash1) failed: 0x%x\n", ret);
            
        if ((R_FAILED(ret = sceIoUnassign("flash2:"))) && (ret != 0x80020321))
            Log::Error("sceIoUnassign(flash2) failed: 0x%x\n", ret);
            
        if (R_FAILED(ret = sceIoAssign("flash2:", "lflash0:0,2", "flashfat2:", IOASSIGN_RDONLY, nullptr, 0)))
            Log::Error("sceIoAssign(flash2) failed: 0x%x\n", ret);
            
        if ((R_FAILED(ret = sceIoUnassign("flash3:"))) && (ret != 0x80020321))
            Log::Error("sceIoUnassign(flash3) failed: 0x%x\n", ret);
            
        if (R_FAILED(ret = sceIoAssign("flash3:", "lflash0:0,3", "flashfat3:", IOASSIGN_RDONLY, nullptr, 0)))
            Log::Error("sceIoAssign(flash3) failed: 0x%x\n", ret);
    }

    bool Init(char *string) {
        int ret = 0;
        char psp_ip[16];
        unsigned short int psp_port = 0;

        scePowerLock(0);
        sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
        sceUtilityLoadNetModule(PSP_NET_MODULE_INET);

        if (R_FAILED(ret = FTP::InitNet())) {
            std::sprintf(string, "Net initialization Failed.");
            return false;
        }
        
        FTP::DisplayNetDialog();

        ret = ftppsp_init(psp_ip, &psp_port);
        if (is_psp_go) {
            if (is_ms_inserted) {
                ftppsp_add_device("ms0:");
                ftppsp_add_device("ef0:");
            }
            else
                ftppsp_add_device("ef0:");
        }
        else
            ftppsp_add_device("ms0:");

        if (cfg.dev_options) {
            FTP::InitFlash();
            ftppsp_add_device("flash0:");
            ftppsp_add_device("flash1:");
            ftppsp_add_device("flash2:");
            ftppsp_add_device("flash3:");
        }
            
        if (ret < 0)
            std::sprintf(string, "Connection Failed.");
        else
            std::sprintf(string, "FTP Connected %s:%i", psp_ip, psp_port);

        return true;
    }

    void Exit(void) {
        ftppsp_fini();
        
        if (cfg.dev_options)
            FTP::ExitFlash();
        
        FTP::ExitNet();
        
        sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
        sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
        scePowerUnlock(0);
    }
}

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
    static char ftp_text[48];

    static void DisplayFTPSettings(void) {
        G2D::DrawRect(0, 18, 480, 254, G2D_RGBA(0, 0, 0, cfg.dark_theme? 50: 80));
        G2D::DrawImage(cfg.dark_theme? dialog_dark : dialog, ((480 - (dialog->w)) / 2), ((272 - (dialog->h)) / 2));
        G2D::FontSetStyle(font, 1.0f, TITLE_COLOUR, INTRAFONT_ALIGN_LEFT);
        intraFontPrint(font, ((480 - (dialog->w)) / 2) + 10, ((272 - (dialog->h)) / 2) + 20, "FTP");

        int ok_width = intraFontMeasureText(font, "OK");
        G2D::DrawRect((409 - (ok_width)) - 5, (180 - (font->texYSize - 15)) - 5, ok_width + 10, (font->texYSize - 5) + 10, SELECTOR_COLOUR);
        intraFontPrint(font, 409 - (ok_width), (192 - (font->texYSize - 15)) - 3, "OK");
        
        int text_width = intraFontMeasureText(font, ftp_text);
        G2D::FontSetStyle(font, 1.0f, TEXT_COLOUR, INTRAFONT_ALIGN_LEFT);
        intraFontPrint(font, ((480 - (text_width)) / 2), ((272 - (dialog->h)) / 2) + 60, ftp_text);
    }

    static void ControlFTPSettings(void) {
        if (Utils::IsButtonPressed(PSP_CTRL_CANCEL)) {
            FTP::Exit();
            settings_state = GENERAL_SETTINGS;
        }

        Utils::SetBounds(&selection, 0, 0);
    }

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

        Utils::SetBounds(&selection, 0, 3);
    }

    static void DisplayAboutSettings(void) {
        G2D::DrawRect(0, 18, 480, 254, G2D_RGBA(0, 0, 0, cfg.dark_theme? 50: 80));
        G2D::DrawImage(cfg.dark_theme? dialog_dark : dialog, ((480 - (dialog->w)) / 2), ((272 - (dialog->h)) / 2));
        G2D::FontSetStyle(font, 1.0f, TITLE_COLOUR, INTRAFONT_ALIGN_LEFT);
        intraFontPrint(font, ((480 - (dialog->w)) / 2) + 10, ((272 - (dialog->h)) / 2) + 20, "About");

        int ok_width = intraFontMeasureText(font, "OK");
        G2D::DrawRect((409 - (ok_width)) - 5, (180 - (font->texYSize - 15)) - 5, ok_width + 10, (font->texYSize - 5) + 10, SELECTOR_COLOUR);
        intraFontPrint(font, 409 - (ok_width), (192 - (font->texYSize - 15)) - 3, "OK");
        
        G2D::FontSetStyle(font, 1.0f, TEXT_COLOUR, INTRAFONT_ALIGN_LEFT);
        int version_width = intraFontMeasureText(font, "CMFileManager-PSP version: v4.0.0");
        intraFontPrintf(font, ((480 - (version_width)) / 2), ((272 - (dialog->h)) / 2) + 50, "CMFileManager-PSP version: v%d.%d.%d", 
            VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);

        int author_width = intraFontMeasureText(font, "Author: Joel16");
        intraFontPrint(font, ((480 - (author_width)) / 2), ((272 - (dialog->h)) / 2) + 68, "Author: Joel16");
    }

    static void ControlAboutSettings(void) {
        if ((Utils::IsButtonPressed(PSP_CTRL_ENTER)) || (Utils::IsButtonPressed(PSP_CTRL_CANCEL)))
            settings_state = GENERAL_SETTINGS;
        
        Utils::SetBounds(&selection, 4, 4);
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
                case 0:
                    FTP::Init(ftp_text);
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
