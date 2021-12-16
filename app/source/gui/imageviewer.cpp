#include "config.h"
#include "fs.h"
#include "colours.h"
#include "g2d.h"
#include "gui.h"
#include "textures.h"
#include "utils.h"

namespace ImageViewer {
    static void Draw(g2dTexture *tex, float w, float h, float zoom_factor, int angle, int pos_x, int pos_y) {
        g2dBeginRects(tex); {
            g2dSetCoordMode(G2D_CENTER);
            g2dSetScaleWH(w * zoom_factor, h * zoom_factor);
            g2dSetCoordXY((G2D_SCR_W / 2) - (pos_x * zoom_factor - pos_x) / 2, (G2D_SCR_H / 2) - (pos_y * zoom_factor - pos_y) / 2);
            g2dSetRotation(angle);
            g2dAdd();
        }
        g2dEnd();
    }
}

namespace GUI {
    static bool properties = false, horizantal_flip = false, vertical_flip = false;
    static float scale_factor = 1.f, width = 0.f, height = 0.f, zoom_factor = 1.f;
    static int degrees = 0, pos_x = 0, pos_y = 0;

    void DisplayImageViewer(MenuItem *item) {
        g2dClear(BLACK_BG);
        
        if (static_cast<float>(item->texture->h) > 272.f) {
            scale_factor = (272.f / static_cast<float>(item->texture->h));
            width = static_cast<float>(item->texture->w) * scale_factor;
            height = static_cast<float>(item->texture->h) * scale_factor;
        }
        else {
            width = static_cast<float>(item->texture->w) * scale_factor;
            height = static_cast<float>(item->texture->h) * scale_factor;
        }

        ImageViewer::Draw(item->texture, width, height, zoom_factor, degrees, pos_x, pos_y);

        if (properties) {
            G2D::DrawRect(0, 0, 480, 272, G2D_RGBA(0, 0, 0, cfg.dark_theme? 50 : 80));
            G2D::DrawImage(properties_dialog[cfg.dark_theme], ((480 - (properties_dialog[0]->w)) / 2), ((272 - (properties_dialog[0]->h)) / 2));
            G2D::FontSetStyle(1.f, TITLE_COLOUR, INTRAFONT_ALIGN_LEFT);
            
            G2D::DrawText(((480 - (properties_dialog[0]->w)) / 2) + 10, ((272 - (properties_dialog[0]->h)) / 2) + 20, "Properties");
            
            int ok_width = intraFontMeasureText(font, "OK");
            G2D::DrawRect((340 - (ok_width)) - 5, (220 - (font->texYSize - 15)) - 5, ok_width + 10, (font->texYSize - 5) + 10, SELECTOR_COLOUR);
            G2D::DrawText(340 - (ok_width), (232 - (font->texYSize - 15)) - 3, "OK");
            
            G2D::FontSetStyle(1.f, TEXT_COLOUR, INTRAFONT_ALIGN_LEFT);
            intraFontPrintf(font, 140, 74, std::string(item->entries[item->selected].d_name).length() > 14? "Name: %.14s..." : "%s", 
                item->entries[item->selected].d_name);
            intraFontPrintf(font, 140, 92, "Width: %dpx", item->texture->w);
            intraFontPrintf(font, 140, 110, "Height: %dpx", item->texture->h);
        }
    }

    void ControlImageViewer(MenuItem *item, float *delta_time) {
        if (Utils::IsButtonPressed(PSP_CTRL_TRIANGLE))
            properties = !properties;
        
        if (Utils::IsButtonPressed(PSP_CTRL_LTRIGGER)) {
            degrees -= 90;
            
            if (degrees < 0)
                degrees = 270;
        }
        else if (Utils::IsButtonPressed(PSP_CTRL_RTRIGGER)) {
            degrees += 90;
            
            if (degrees > 270)
                degrees = 0;
        }

        // Flip horizantally
        if (Utils::IsButtonPressed(PSP_CTRL_SQUARE)) {
            horizantal_flip = !horizantal_flip;
            width = -width;
        }
        
        // Flip vertically
        if (Utils::IsButtonPressed(PSP_CTRL_TRIANGLE)) {
            vertical_flip = !vertical_flip;
            height = -height;
        }
        
        if (Utils::IsButtonHeld(PSP_CTRL_UP)) {
            zoom_factor += 0.5f * (*delta_time);
            
            if (zoom_factor > 2.0f)
                zoom_factor = 2.0f;
        }
        else if (Utils::IsButtonHeld(PSP_CTRL_DOWN)) {
            zoom_factor -= 0.5f * (*delta_time);
            
            if (zoom_factor < 0.5f)
                zoom_factor = 0.5f;
                
            if (zoom_factor <= 1.0f) {
                pos_x = 0;
                pos_y = 0;
            }
        }

        if ((height * zoom_factor > 272.f) || (width * zoom_factor > 480.f)) {
            double velocity = 2.f / zoom_factor;
            if (Utils::GetAnalogY() < -0.4f)
                pos_y -= ((velocity * zoom_factor) * (*delta_time) * 1000.f);
            if (Utils::GetAnalogY() > 0.4f)
                pos_y += ((velocity * zoom_factor) * (*delta_time) * 1000.f);
            if (Utils::GetAnalogX() < -0.4f)
                pos_x -= ((velocity * zoom_factor) * (*delta_time) * 1000.f);
            if (Utils::GetAnalogX() > 0.4f)
                pos_x += ((velocity * zoom_factor) * (*delta_time) * 1000.f);
        }
        
        if ((degrees == 0) || (degrees == 180)) {
            Utils::SetMax(pos_x, horizantal_flip? -width : width, horizantal_flip? -width : width);
            Utils::SetMin(pos_x, horizantal_flip? width : -width, horizantal_flip? width : -width);
            Utils::SetMax(pos_y, vertical_flip? -height : height, vertical_flip? -height : height);
            Utils::SetMin(pos_y, vertical_flip? height : -height, vertical_flip? height : -height);
        }
        else {
            Utils::SetMax(pos_x, vertical_flip? -height : height, vertical_flip? -height : height);
            Utils::SetMin(pos_x, vertical_flip? height : -height, vertical_flip? height : -height);
            Utils::SetMax(pos_y, horizantal_flip? -width : width, horizantal_flip? -width : width);
            Utils::SetMin(pos_y, horizantal_flip? width : -width, horizantal_flip? width : -width);
        }

        if (Utils::IsButtonPressed(PSP_CTRL_CANCEL)) {
            if (!properties) {
                if (item->texture)
                    g2dTexFree(&item->texture);
                
                zoom_factor = 1.f;
                pos_x = 0;
                pos_y = 0;
                item->state = MENU_STATE_FILEBROWSER;
            }
            else
                properties = false;
        }

        if (Utils::IsButtonPressed(PSP_CTRL_ENTER)) {
            if (properties)
                properties = false;
        }
    }
}
