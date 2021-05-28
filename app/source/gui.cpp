#include <psppower.h>
#include <sys/time.h>
#include <psprtc.h>

#include "config.h"
#include "colours.h"
#include "fs.h"
#include "g2d.h"
#include "gui.h"
#include "textures.h"
#include "utils.h"

namespace GUI {
    void ResetCheckbox(MenuItem *item) {
        item->checked.clear();
        item->checked_copy.clear();
        item->checked.resize(item->entries.size());
        item->checked.assign(item->checked.size(), false);
        item->checked_cwd.clear();
        item->checked_count = 0;
    };

    void GetStorageSize(MenuItem *item) {
        item->total_storage = Utils::GetTotalStorage();
        item->used_storage = Utils::GetUsedStorage();
    }

    void DisplayStatusBar(void) {
        pspTime time;
        static char time_string[30];

        if (R_SUCCEEDED(sceRtcGetCurrentClockLocalTime(&time)))
            snprintf(time_string, 30, "%2i:%02i %s", ((time.hour % 12) == 0)? 12 : time.hour % 12, time.minutes, (time.hour / 12)? "PM" : "AM");

        G2D::FontSetStyle(font, 1.0f, WHITE, INTRAFONT_ALIGN_LEFT);
        intraFontPrint(font, 5, 14, time_string);

        int state = scePowerIsBatteryCharging();
        int percent = scePowerGetBatteryLifePercent();
        int battery_val = (percent / 20);
        
        static char percent_string[13];
        snprintf(percent_string, 13, "%d%%", percent);
        int percent_width = intraFontMeasureText(font, percent_string); 
        intraFontPrint(font, 475 - percent_width, 14, percent_string);

        G2D::DrawImage(state != 0? battery_charging[battery_val] : battery[battery_val], 475 - percent_width - battery[battery_val]->w, 2);
    }

    int RenderLoop(void) {
        bool done = false;
        MenuItem item;

        int ret = 0;
        if (R_FAILED(ret = FS::GetDirList(cfg.cwd, item.entries)))
            return ret;

        GUI::ResetCheckbox(&item);
        GUI::GetStorageSize(&item);
        
        u64 last = 0;
        u32 tick = sceRtcGetTickResolution();
        sceRtcGetCurrentTick(&last);

        Colours::Get();

        while(!done) {
            u64 current = 0;
            sceRtcGetCurrentTick(&current);
            
            float delta_time = (current - last) / static_cast<float>(tick);
            last = current;
            
            int ctrl = Utils::ReadControls();
            
            g2dClear(BG_COLOUR);
            G2D::DrawRect(0, 0, 480, 18, STATUS_BAR_COLOUR);
            G2D::DrawRect(0, 18, 480, 34, MENU_BAR_COLOUR);
            G2D::DrawImageScale(icon_nav_drawer, 5, 24, 26.f, 26.f);
            GUI::DisplayStatusBar();
            GUI::DisplayFileBrowser(&item);

            switch(item.state) {
                case MENU_STATE_FILEBROWSER:
                    GUI::ControlFileBrowser(&item, &ctrl);
                    break;
                
                case MENU_STATE_OPTIONS:
                    GUI::DisplayFileOptions(&item);
                    GUI::ControlFileOptions(&item, &ctrl);
                    break;

                case MENU_STATE_PROPERTIES:
                    GUI::DisplayFileProperties(&item);
                    GUI::ControlFileProperties(&item);
                    break;

                case MENU_STATE_DELETE:
                    GUI::DisplayDeleteOptions(&item);
                    GUI::ControlDeleteOptions(&item, &ctrl);
                    break;

                case MENU_STATE_SETTINGS:
                    GUI::DisplaySettings(&item);
                    GUI::ControlSettings(&item, &ctrl);
                    break;

                case MENU_STATE_IMAGEVIEWER:
                    GUI::DisplayImageViewer(&item);
                    GUI::ControlImageViewer(&item, &delta_time);
                    break;

                default:
                    break;
            }

            g2dFlip(G2D_VSYNC);

            if (Utils::IsButtonPressed(PSP_CTRL_START))
                item.state = MENU_STATE_SETTINGS;
            else if (Utils::IsKButtonPressed(PSP_CTRL_HOME))
                break;
        }

        return 0;
    }
}
