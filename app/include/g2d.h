#ifndef _CMFILEMANAGER_GLIB2D_HELPER_H_
#define _CMFILEMANAGER_GLIB2D_HELPER_H_

#include <glib2d.h>
#include <intraFont.h>
#include <string>

extern intraFont *font, *jpn0, *chn;
extern char font_size_cache[256];

namespace G2D {
    void DrawRect(float x, float y, float width, float height, g2dColor colour);
    void DrawImage(g2dTexture *tex, float x, float y);
    void DrawImageScale(g2dTexture *tex, float x, float y, float w, float h);
    char *KeyboardGetText(const std::string &desc_msg, const std::string &initial_msg);
    void FontSetStyle(intraFont *font, float size, unsigned int colour, unsigned int options);
    float GetTextHeight(intraFont *font);
    float DrawText(float x, float y, const char *text);
}

#endif
