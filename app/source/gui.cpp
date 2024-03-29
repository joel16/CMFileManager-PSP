#include <psppower.h>
#include <sys/time.h>
#include <psprtc.h>

#include "colours.h"
#include "config.h"
#include "fs.h"
#include "g2d.h"
#include "gui.h"
#include "screenshot.h"
#include "textures.h"
#include "utils.h"

namespace GUI {
    static MenuItem item;

    void ResetCheckbox(MenuItem &item) {
        item.checked.clear();
        item.checked_copy.clear();
        item.checked.resize(item.entries.size());
        item.checked.assign(item.checked.size(), false);
        item.checked_cwd.clear();
        item.checked_count = 0;
    };

    void GetStorageSize(MenuItem &item) {
        item.total_storage = Utils::GetTotalStorage();
        item.used_storage = Utils::GetUsedStorage();
    }

    void DisplayStatusBar(void) {
        pspTime time;
        static char time_string[30];

        if (R_SUCCEEDED(sceRtcGetCurrentClockLocalTime(&time)))
            std::snprintf(time_string, 30, "%2i:%02i %s", ((time.hour % 12) == 0)? 12 : time.hour % 12, time.minutes, (time.hour / 12)? "PM" : "AM");

        G2D::FontSetStyle(1.f, WHITE, INTRAFONT_ALIGN_LEFT);
        G2D::DrawText(5, 14, time_string);

        int state = 0, percent = 0, battery_val = 0;
        if (scePowerIsBatteryExist()) {
            state = scePowerIsBatteryCharging();
            percent = scePowerGetBatteryLifePercent();
            battery_val = (percent / 20);
        }
        
        static char percent_string[10];
        std::snprintf(percent_string, 10, "%d", percent);
        int percent_width = intraFontMeasureText(font, percent_string);
        intraFontPrintf(font, 475 - percent_width - 15, 14, "%s%%", percent_string);

        G2D::DrawImage(state != 0? battery_charging[battery_val] : battery[battery_val], 475 - percent_width - battery[battery_val]->w - 15, 2);
    }

    void ProgressBar(const std::string &title, std::string message, u64 offset, u64 size) {
        if (message.length() > 35) {
            message.resize(35);
            message.append("...");
        }
        
        g2dClear(BG_COLOUR);
        G2D::DrawRect(0, 0, 480, 18, STATUS_BAR_COLOUR);
        G2D::DrawRect(0, 18, 480, 34, MENU_BAR_COLOUR);
        G2D::DrawImageScale(icon_nav_drawer, 5, 24, 26.f, 26.f);
        GUI::DisplayStatusBar();
        GUI::DisplayFileBrowser(item);
        
        G2D::DrawRect(0, 18, 480, 254, G2D_RGBA(0, 0, 0, cfg.dark_theme? 50 : 80));
        G2D::DrawImage(dialog[cfg.dark_theme], ((480 - (dialog[0]->w)) / 2), ((272 - (dialog[0]->h)) / 2));
        G2D::FontSetStyle(1.f, TITLE_COLOUR, INTRAFONT_ALIGN_LEFT);
        G2D::DrawText(((480 - (dialog[0]->w)) / 2) + 10, ((272 - (dialog[0]->h)) / 2) + 20, title.c_str());

        int text_width = intraFontMeasureText(font, message.c_str());
        G2D::FontSetStyle(1.f, TEXT_COLOUR, INTRAFONT_ALIGN_LEFT);
        G2D::DrawText(((480 - (text_width)) / 2), ((272 - (dialog[0]->h)) / 2) + 60, message.c_str());
        
        G2D::DrawRect(((480 - dialog[0]->w) / 2) + 20, ((272 - dialog[0]->h) / 2) + 70, 318, 4, SELECTOR_COLOUR);
        G2D::DrawRect(((480 - dialog[0]->w) / 2) + 20, ((272 - dialog[0]->h) / 2) + 70,
            static_cast<int>((static_cast<float>(offset) / static_cast<float>(size)) * 318.f), 4, TITLE_COLOUR);
        
        g2dFlip(G2D_VSYNC);
    }

    int RenderLoop(void) {
        int ret = 0;
        if (R_FAILED(ret = FS::GetDirList(cfg.cwd, item.entries)))
            return ret;

        GUI::ResetCheckbox(item);
        GUI::GetStorageSize(item);

        Colours::Get();
        
        // Delta time
        u64 last_tick = 0;
        float resolution = sceRtcGetTickResolution() / 1000.f;
        sceRtcGetCurrentTick(&last_tick);

        while(g_running) {
            u64 current_tick = 0;
            sceRtcGetCurrentTick(&current_tick);
            float delta = (current_tick - last_tick) / resolution;
            last_tick = current_tick;

            if (delta < 0.001f)
                delta = 0.001f;
            
            int ctrl = Utils::ReadControls();
            
            g2dClear(BG_COLOUR);
            G2D::DrawRect(0, 0, 480, 18, STATUS_BAR_COLOUR);
            G2D::DrawRect(0, 18, 480, 34, MENU_BAR_COLOUR);
            GUI::DisplayStatusBar();
            G2D::DrawImageScale(icon_nav_drawer, 5, 24, 26.f, 26.f);
            GUI::DisplayFileBrowser(item);

            switch(item.state) {
                case MENU_STATE_MENUBAR:
                    GUI::HandleMenubarAnim(delta);
                    GUI::DisplayMenubar();
                    GUI::ControlMenubar(item, ctrl);
                    break;
                
                case MENU_STATE_FILEBROWSER:
                    GUI::ControlFileBrowser(item, ctrl);
                    break;
                
                case MENU_STATE_OPTIONS:
                    GUI::DisplayFileOptions(item);
                    GUI::ControlFileOptions(item, ctrl);
                    break;

                case MENU_STATE_PROPERTIES:
                    GUI::DisplayFileProperties(item);
                    GUI::ControlFileProperties(item);
                    break;

                case MENU_STATE_DELETE:
                    GUI::DisplayDeleteOptions();
                    GUI::ControlDeleteOptions(item, ctrl);
                    break;

                case MENU_STATE_SETTINGS:
                    GUI::DisplaySettings(item);
                    GUI::ControlSettings(item, ctrl);
                    break;

                case MENU_STATE_IMAGEVIEWER:
                    GUI::DisplayImageViewer(item);
                    GUI::ControlImageViewer(item, delta);
                    break;

                default:
                    break;
            }

            g2dFlip(G2D_VSYNC);
            
            if (Utils::IsButtonPressed(static_cast<enum PspCtrlButtons>(PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER)))
                Screenshot::Capture();

            if (Utils::IsButtonPressed(PSP_CTRL_START))
                item.state = MENU_STATE_SETTINGS;
        }

        return 0;
    }
}
