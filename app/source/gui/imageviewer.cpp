#include "config.h"
#include "fs.h"
#include "colours.h"
#include "g2d.h"
#include "gui.h"
#include "textures.h"
#include "utils.h"

namespace ImageViewer {
    static void Draw(g2dTexture *tex, float w, float h, float zoom, int angle, int posX, int posY) {
        g2dBeginRects(tex); {
            g2dSetCoordMode(G2D_CENTER);
            g2dSetScaleWH(w * zoom, h * zoom);
            g2dSetCoordXY((G2D_SCR_W / 2) - (posX * zoom - posX) / 2, (G2D_SCR_H / 2) - (posY * zoom - posY) / 2);
            g2dSetRotation(angle);
            g2dAdd();
        }
        g2dEnd();
    }
}

namespace GUI {
    static bool properties = false;
    static float scale = 1.f, width = 0.f, height = 0.f, zoom = 1.f;
    static int degrees = 0, posX = 0, posY = 0;

    void DisplayImageViewer(MenuItem &item) {
        g2dClear(BLACK_BG);
        
        if (static_cast<float>(item.texture->h) > 272.f) {
            scale = (272.f / static_cast<float>(item.texture->h));
            width = static_cast<float>(item.texture->w) * scale;
            height = static_cast<float>(item.texture->h) * scale;
        }
        else {
            width = static_cast<float>(item.texture->w) * scale;
            height = static_cast<float>(item.texture->h) * scale;
        }

        ImageViewer::Draw(item.texture, width, height, zoom, degrees, posX, posY);

        if (properties) {
            G2D::DrawRect(0, 0, 480, 272, G2D_RGBA(0, 0, 0, cfg.dark_theme? 50 : 80));
            G2D::DrawImage(properties_dialog[cfg.dark_theme], ((480 - (properties_dialog[0]->w)) / 2), ((272 - (properties_dialog[0]->h)) / 2));
            G2D::FontSetStyle(1.f, TITLE_COLOUR, INTRAFONT_ALIGN_LEFT);
            
            G2D::DrawText(((480 - (properties_dialog[0]->w)) / 2) + 10, ((272 - (properties_dialog[0]->h)) / 2) + 20, "Properties");
            
            int ok_width = intraFontMeasureText(fonts[FONT_DEFAULT], "OK");
            G2D::DrawRect((340 - (ok_width)) - 5, (220 - (fonts[FONT_DEFAULT]->texYSize - 15)) - 5, ok_width + 10, (fonts[FONT_DEFAULT]->texYSize - 5) + 10, SELECTOR_COLOUR);
            G2D::DrawText(340 - (ok_width), (232 - (fonts[FONT_DEFAULT]->texYSize - 15)) - 3, "OK");
            
            G2D::FontSetStyle(1.f, TEXT_COLOUR, INTRAFONT_ALIGN_LEFT);
            intraFontPrintf(fonts[FONT_DEFAULT], 140, 74, std::string(item.entries[item.selected].d_name).length() > 14? "Name: %.14s..." : "%s", 
                item.entries[item.selected].d_name);
            intraFontPrintf(fonts[FONT_DEFAULT], 140, 92, "Width: %dpx", item.texture->w);
            intraFontPrintf(fonts[FONT_DEFAULT], 140, 110, "Height: %dpx", item.texture->h);
        }
    }

    void ControlImageViewer(MenuItem &item, float &delta) {
        if (Utils::IsButtonPressed(PSP_CTRL_LTRIGGER)) {
            degrees -= 90;
            
            if (degrees < 0) {
                degrees = 270;
            }
        }
        else if (Utils::IsButtonPressed(PSP_CTRL_RTRIGGER)) {
            degrees += 90;
            
            if (degrees > 270) {
                degrees = 0;
            }
        }
        
        if (Utils::IsButtonPressed(PSP_CTRL_TRIANGLE)) {
            properties = !properties;
        }
        
        if (Utils::IsButtonHeld(PSP_CTRL_UP)) {
            zoom += (delta / 1000.f);
            
            if (zoom > 2.f) {
                zoom = 2.f;
            }
        }
        else if (Utils::IsButtonHeld(PSP_CTRL_DOWN)) {
            zoom -= (delta / 1000.f);
            
            if (zoom < 0.5f) {
                zoom = 0.5f;
            }
                
            if (zoom <= 1.f) {
                posX = 0;
                posY = 0;
            }
        }

        if ((height * zoom > 272.f) || (width * zoom > 480.f)) {
            float velocity = 2.f / zoom;

            if (Utils::GetAnalogY() < -0.4f) {
                posY -= ((velocity * zoom) * delta);
            }
            if (Utils::GetAnalogY() > 0.4f) {
                posY += ((velocity * zoom) * delta);
            }
            if (Utils::GetAnalogX() < -0.4f) {
                posX -= ((velocity * zoom) * delta);
            }
            if (Utils::GetAnalogX() > 0.4f) {
                posX += ((velocity * zoom) * delta);
            }
        }
        
        if ((degrees == 0) || (degrees == 180)) {
            Utils::SetMax(posX, width, width);
            Utils::SetMin(posX, -width, -width);
            Utils::SetMax(posY, height, height);
            Utils::SetMin(posY, -height, -height);
        }
        else {
            Utils::SetMax(posX, height, height);
            Utils::SetMin(posX, -height, -height);
            Utils::SetMax(posY, width, width);
            Utils::SetMin(posY, -width, -width);
        }

        if (Utils::IsButtonPressed(PSP_CTRL_CANCEL)) {
            if (!properties) {
                if (item.texture) {
                    g2dTexFree(&item.texture);
                }
                
                zoom = 1.f;
                posX = 0;
                posY = 0;
                item.state = MENU_STATE_FILEBROWSER;
            }
            else {
                properties = false;
            }
        }

        if (Utils::IsButtonPressed(PSP_CTRL_ENTER)) {
            if (properties) {
                properties = false;
            }
        }
    }
}
