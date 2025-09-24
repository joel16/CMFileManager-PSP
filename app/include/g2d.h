#pragma once

#include <glib2d.h>
#include <intraFont.h>
#include <string>

enum FontType {
    FONT_DEFAULT, // ltn8.pgf (Latin)
    FONT_JPN,     // jpn0.pgf (Japanese)
    FONT_CN,      // gb3s1518.bwfon (Chinese)
    FONT_KOR,     // kr0.pgf (Korean)
    FONT_SYM,     // arib.pgf (Symbols)
    NUM_FONTS
};

extern intraFont *fonts[NUM_FONTS];
extern char font_size_cache[256];

namespace G2D {
    void DrawRect(float x, float y, float width, float height, g2dColor colour);
    void DrawImage(g2dTexture *tex, float x, float y);
    void DrawImageScale(g2dTexture *tex, float x, float y, float w, float h);
    char *KeyboardGetText(const std::string &desc_msg, const std::string &initialMsg);
    int LoadFonts(void);
    void UnloadFonts(void);
    void FontSetStyle(float size, unsigned int colour, unsigned int options);
    float GetTextHeight(void);
    float DrawText(float x, float y, const std::string &text);
}
