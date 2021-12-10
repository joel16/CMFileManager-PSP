#include <algorithm>
#include <cstdio>

#include "audio.h"
#include "colours.h"
#include "config.h"
#include "fs.h"
#include "g2d.h"
#include "gui.h"
#include "kernel_functions.h"
#include "textures.h"
#include "utils.h"

namespace AudioPlayer {
    typedef enum {
        STATE_NONE,
        STATE_REPEAT,
        STATE_SHUFFLE
    } AudioState;
    
    static AudioState state = STATE_NONE;
    
    static void SecsToString(char *string, u64 seconds) {
        int h = 0, m = 0, s = 0;
        h = (seconds / 3600);
        m = (seconds - (3600 * h)) / 60;
        s = (seconds - (3600 * h) - (m * 60));
        
        if (h > 0)
            snprintf(string, 35, "%02d:%02d:%02d", h, m, s);
        else
            snprintf(string, 35, "%02d:%02d", m, s);
    }
    
    void Play(const std::string &path) {
        Audio::Init(path);
        
        char *position_time = new char[35];
        char *length_time = new char[35];
        float length_time_width = 0;
        
        AudioPlayer::SecsToString(length_time, Audio::GetLengthSeconds());
        G2D::FontSetStyle(font, 1.0f, WHITE, INTRAFONT_ALIGN_LEFT);
        length_time_width = intraFontMeasureText(font, length_time);
        
        bool screen_disabled = false;
        
        std::string filename = FS::GetFilename(path);
        std::transform(filename.begin(), filename.end(), filename.begin(), ::toupper);
        
        while(playing) {
            g2dClear(cfg.dark_theme? BLACK_BG : WHITE);
            G2D::DrawImage(default_artwork_blur, 0, 0);
            G2D::DrawRect(0, 0, 480, 20, G2D_RGBA(97, 97, 97, 255));
            
            G2D::DrawImage(icon_back, 5, 25);
            GUI::DisplayStatusBar();
            
            if ((metadata.has_meta) && (metadata.title.c_str()[0] != '\0') && (metadata.artist.c_str()[0] != '\0')) {
                std::transform(metadata.title.begin(), metadata.title.end(), metadata.title.begin(), ::toupper);
                G2D::DrawText(40, 10 + ((40 - (font->texYSize - 30)) / 2), metadata.title.c_str());
                std::transform(metadata.artist.begin(), metadata.artist.end(), metadata.artist.begin(), ::toupper);
                G2D::DrawText(40, 25 + ((40 - (font->texYSize - 30)) / 2), metadata.artist.c_str());
            }
            else if ((metadata.has_meta) && (metadata.title.c_str()[0] != '\0')) {
                std::transform(metadata.title.begin(), metadata.title.end(), metadata.title.begin(), ::toupper);
                G2D::DrawText(40, 16 + ((40 - (font->texYSize - 30)) / 2), metadata.title.c_str());
            }
            else
                G2D::DrawText(40, 16 + ((40 - (font->texYSize - 30)) / 2), filename.c_str());
                
            G2D::DrawRect(0, 62, 200, 200, G2D_RGBA(97, 97, 97, 255));
            
            if ((metadata.has_meta) && (metadata.cover_image))
                G2D::DrawImageScale(metadata.cover_image, 0, 62, 200, 200);
            else
                G2D::DrawImage(default_artwork, 0, 62); // Default album art
                
            G2D::DrawRect(205, 62, 275, 200, G2D_RGBA(45, 48, 50, 255)); // Draw info box (outer)
            G2D::DrawRect(210, 67, 265, 190, G2D_RGBA(46, 49, 51, 255)); // Draw info box (inner)
            
            if (!Audio::IsPaused())
                G2D::DrawImage(btn_pause, 205 + ((275 - btn_pause->w) / 2), 62 + ((200 - btn_pause->h) / 2)); // Playing
            else
                G2D::DrawImage(btn_play, 205 + ((275 - btn_play->w) / 2), 62 + ((200 - btn_play->h) / 2)); // Paused
                
            G2D::DrawImage(btn_rewind, 205 + ((275 - btn_rewind->w) / 2) - 68, 62 + ((200 - btn_rewind->h) / 2));
            G2D::DrawImage(btn_forward, 205 + ((275 - btn_forward->w) / 2) + 68, 62 + ((200 - btn_forward->h) / 2));
            
            G2D::DrawImage(state == STATE_SHUFFLE? btn_shuffle_overlay : btn_shuffle, 205 + ((275 - btn_shuffle->w) / 2) - 45, 62 + ((200 - btn_shuffle->h) / 2) + 50);
            G2D::DrawImage(state == STATE_REPEAT? btn_repeat_overlay : btn_repeat, 205 + ((275 - btn_repeat->w) / 2) + 45, 62 + ((200 - btn_repeat->h) / 2) + 50);
            
            AudioPlayer::SecsToString(position_time, Audio::GetPositionSeconds());
            G2D::DrawText(230, 240, position_time);
            G2D::DrawText(455 - length_time_width, 240, length_time);
            
            G2D::DrawRect(230, 245, 225, 2, G2D_RGBA(97, 97, 97, 150));
            G2D::DrawRect(230, 245, ((static_cast<double>(Audio::GetPosition())/static_cast<double>(Audio::GetLength())) * 225.0), 2, WHITE);
            
            g2dFlip(G2D_VSYNC);
            Utils::ReadControls();
            
            if (Utils::IsButtonPressed(PSP_CTRL_SELECT)) {
                screen_disabled = !screen_disabled;
                
                if (screen_disabled)
                    pspDisplayDisable();
                else
                    pspDisplayEnable();
            }
            
            if (Utils::IsButtonPressed(PSP_CTRL_ENTER))
                Audio::Pause();
                
            if (Utils::IsButtonPressed(PSP_CTRL_CANCEL))
                Audio::Stop();
        }
        
        delete[] length_time;
        delete[] position_time;
        Audio::Exit();
        
        // If user tries to exit with screen disabled, enable it.
        if (screen_disabled)
            pspDisplayEnable();
    }
}
