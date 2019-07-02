#include <stdio.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "glib2d_helper.h"
#include "status_bar.h"
#include "textures.h"

static char message_resized[41];

static void Dialog_DisplayBoxAndMsg(const char *title, const char *msg_1, const char *msg_2, int msg_1_width, int msg_2_width, bool with_bg) {
    if (with_bg) {
        g2dClear(config.dark_theme? BLACK_BG : WHITE);
        G2D_DrawRect(0, 0, 480, 20, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);
        G2D_DrawRect(0, 20, 480, 42, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
        G2D_DrawImage(icon_nav_drawer, 5, 25);
        
        StatusBar_DisplayTime();
        Dirbrowse_DisplayFiles();
    }
    G2D_DrawRect(0, 20, 480, 252, G2D_RGBA(0, 0, 0, config.dark_theme? 50: 80));
    
    G2D_DrawImage(config.dark_theme? dialog_dark : dialog, ((480 - dialog->w) / 2), ((272 - dialog->h) / 2));
    intraFontSetStyle(font, 0.7f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
    intraFontPrint(font, ((480 - dialog->w) / 2) + 10, ((272 - dialog->h) / 2) + 20, title);
    
    intraFontSetStyle(font, 0.7f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
    if (msg_1 && msg_2) {
        intraFontPrint(font, ((480 - (msg_1_width)) / 2), ((272 - dialog->h) / 2) + 50, msg_1);
        intraFontPrint(font, ((480 - (msg_2_width)) / 2), ((272 - dialog->h) / 2) + 50 + 16, msg_2);
    }
    else if (msg_1 && !msg_2)
        intraFontPrint(font, ((480 - (msg_1_width)) / 2), ((272 - dialog->h) / 2) + 60, msg_1);
        
    intraFontSetStyle(font, 0.7f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
}

void Dialog_DisplayMessage(const char *title, const char *msg_1, const char *msg_2) {
    int text_width1 = 0, text_width2 = 0;
    
    if (msg_1)
        text_width1 = intraFontMeasureText(font, msg_1);
        
    if (msg_2)
        text_width2 = intraFontMeasureText(font, msg_2);
    
    Dialog_DisplayBoxAndMsg(title, msg_1, msg_2, text_width1, text_width2, true);

    intraFontSetStyle(font, 0.7f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
    G2D_DrawRect((409 - (intraFontMeasureText(font, "OK"))) - 5, (180 - (font->texYSize - 20)) - 5, intraFontMeasureText(font, "OK") + 10, (font->texYSize - 10) + 10, 
        config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    intraFontPrint(font, 409 - (intraFontMeasureText(font, "OK")), (182 - (font->texYSize - 30)), "OK");
    
    g2dFlip(G2D_VSYNC);
}

void Dialog_DisplayPrompt(const char *title, const char *msg_1, const char *msg_2, int *selection, bool with_bg) {
    int text_width1 = 0, text_width2 = 0;
    
    if (msg_1)
        text_width1 = intraFontMeasureText(font, msg_1);
    if (msg_2)
        text_width2 = intraFontMeasureText(font, msg_2);
        
    Dialog_DisplayBoxAndMsg(title, msg_1, msg_2, text_width1, text_width2, with_bg);
    
    if (*selection == 0)
        G2D_DrawRect((364 - intraFontMeasureText(font, "NO")) - 5, (180 - (font->texYSize - 20)) - 5, intraFontMeasureText(font, "NO") + 10, (font->texYSize - 10) + 10, 
            config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else if (*selection == 1)
        G2D_DrawRect((409 - (intraFontMeasureText(font, "YES"))) - 5, (180 - (font->texYSize - 20)) - 5, intraFontMeasureText(font, "YES") + 10, (font->texYSize - 10) + 10, 
            config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
            
    intraFontPrint(font, 409 - (intraFontMeasureText(font, "YES")), (182 - (font->texYSize - 30)), "YES");
    intraFontPrint(font, 364 - intraFontMeasureText(font, "NO"), (182 - (font->texYSize - 30)), "NO");
    
    if (with_bg)
        g2dFlip(G2D_VSYNC);
}

void Dialog_DisplayProgress(const char *title, const char *message, u32 offset, u32 size) {
    snprintf(message_resized, 41, "%.40s", message);
    int text_width = intraFontMeasureText(font, message_resized);
    
    Dialog_DisplayBoxAndMsg(title, message_resized, NULL, text_width, 0, true);
    
    G2D_DrawRect(((480 - dialog->w) / 2) + 20, ((272 - dialog->h) / 2) + 70, 
        318, 4, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    G2D_DrawRect(((480 - dialog->w) / 2) + 20, ((272 - dialog->h) / 2) + 70, 
        ((double)offset / (double)size * 318.0), 4, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);
        
    g2dFlip(G2D_VSYNC);
}
