#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <psputility.h>
#include <cstring>

#include "g2d.h"
#include "utils.h"

intraFont *font, *jpn0, *chn;

namespace G2D {
    void DrawRect(float x, float y, float width, float height, g2dColor colour) {
        g2dBeginRects(nullptr); {
            g2dSetColor(colour);
            g2dSetScaleWH(width, height);
            g2dSetCoordXY(x, y);
            g2dAdd();
        }
        g2dEnd();
    }

    void DrawImage(g2dTexture *tex, float x, float y) {
        g2dBeginRects(tex); {
            g2dSetCoordXY(x, y);
            g2dAdd();
        }
        g2dEnd();
    }

    void DrawImageScale(g2dTexture *tex, float x, float y, float w, float h) {
        g2dBeginRects(tex); {
            g2dSetScaleWH(w, h);
            g2dSetCoordXY(x, y);
            g2dAdd();
        }
        g2dEnd();
    }

    static int GetText(char *input, unsigned short *intext, unsigned short *desc) {
        bool done = false;
        unsigned short outtext[128] = { 0 };
        
        SceUtilityOskData data;
        std::memset(&data, 0, sizeof(SceUtilityOskData));
        
        data.language = PSP_UTILITY_OSK_LANGUAGE_DEFAULT; // Use system default for text input
        data.lines = 1;
        data.unk_24 = 1;
        data.inputtype = PSP_UTILITY_OSK_INPUTTYPE_ALL; // Allow all input types
        data.desc = desc;
        data.intext = intext;
        data.outtextlength = 128;
        data.outtextlimit = 128; // Limit input to 128 characters
        data.outtext = outtext;
        
        SceUtilityOskParams params;
        memset(&params, 0, sizeof(params));
        
        params.base.size = sizeof(params);
        sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &params.base.language);
        sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &params.base.buttonSwap);
        params.base.graphicsThread = 17;
        params.base.accessThread = 19;
        params.base.fontThread = 18;
        params.base.soundThread = 16;
        params.datacount = 1;
        params.data = &data;
        
        int ret = 0;
        if (R_FAILED(ret = sceUtilityOskInitStart(&params)))
            return ret;
            
        while(!done) {
            int i = 0, j = 0;
            
            g2dClear(G2D_RGBA(39, 50, 56, 255));
            sceGuFinish();
            sceGuSync(0, 0);
            
            switch(sceUtilityOskGetStatus()) {
                case PSP_UTILITY_DIALOG_INIT:
                    break;
                    
                case PSP_UTILITY_DIALOG_VISIBLE:
                    sceUtilityOskUpdate(1);
                    break;
                    
                case PSP_UTILITY_DIALOG_QUIT:
                    sceUtilityOskShutdownStart();
                    break;
                    
                case PSP_UTILITY_DIALOG_FINISHED:
                    break;
                    
                case PSP_UTILITY_DIALOG_NONE:
                    done = true;
                    
                default:
                    break;
            }
            
            for(i = 0; data.outtext[i]; i++) {
                if (data.outtext[i] != '\0' && data.outtext[i] != '\n' && data.outtext[i] != '\r') {
                    input[j] = data.outtext[i];
                    j++;
                }
            }
            
            input[j] = 0;
            g2dFlip(G2D_VSYNC);
        }
        
        return 0;
    }

    char *KeyboardGetText(const std::string &desc_msg, const std::string &initial_msg) {
        int ret = 0;
        size_t i = 0;
        static char str[128];
        unsigned short initial[128]  = { 0 };
        unsigned short desc[128]  = { 0 };
        
        if (initial_msg.c_str()[0] != 0) {
            for (i = 0; i <= initial_msg.length(); i++)
                initial[i] = static_cast<unsigned short>(initial_msg.c_str()[i]);
        }
        
        if (desc_msg.c_str()[0] != 0) {
            for (i = 0; i <= desc_msg.length(); i++)
                desc[i] = static_cast<unsigned short>(desc_msg.c_str()[i]);
        }
        
        if (R_SUCCEEDED(ret = G2D::GetText(str, initial, desc)))
            return str;
            
        return 0;
    }

    void FontSetStyle(intraFont *font, float size, unsigned int colour, unsigned int options) {
        intraFontSetStyle(font, size, colour, G2D_RGBA(0, 0, 0, 0), 0.f, options);
    }

    float GetTextHeight(intraFont *font) {
        return font->advancey * font->size / 4.f + 2.f;
    }

    float DrawText(float x, float y, const char *text) {
        return intraFontPrintf(font, x, y, text);
    }
}
