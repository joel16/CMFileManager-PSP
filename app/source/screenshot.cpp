#include <cstdio>
#include <pspdisplay.h>
#include <pspiofilemgr.h>
#include <psptypes.h>
#include <sys/time.h>
#include <psprtc.h>
#include <string>

#include "fs.h"
#include "libpng/png.h"
#include "utils.h"

namespace Screenshot {
    void WriteFunc(png_structp ptr, png_bytep data, png_size_t length) {
        png_voidp p = png_get_io_ptr(ptr);
        sceIoWrite(*static_cast<SceUID *>(p), data, length);
    }

    void Save(const std::string &path) {
        u32 *vram32 = nullptr;
        u16 *vram16 = nullptr;
        int buf_width = 0, pixel_format = 0;
        const int SCREEN_WIDTH = 480;
        const int SCREEN_HEIGHT = 272;
        
        SceUID file = 0;
        if (R_FAILED(file = sceIoOpen(path.c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777)))
            return;
        
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png_ptr)
            return;
        
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_write_struct(&png_ptr, nullptr);
            sceIoClose(file);
            return;
        }
        
        png_set_write_fn(png_ptr, static_cast<png_voidp>(&file), Screenshot::WriteFunc, nullptr);
        png_set_IHDR(png_ptr, info_ptr, SCREEN_WIDTH, SCREEN_HEIGHT, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);
        
        u8 *line = new u8 [SCREEN_WIDTH * 3];
        sceDisplayWaitVblankStart();
        sceDisplayGetFrameBuf(reinterpret_cast<void **>(&vram32), &buf_width, &pixel_format, 0);
        vram16 = reinterpret_cast<u16 *>(vram32);
        
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int i = 0, x = 0; x < SCREEN_WIDTH; x++) {
                u32 color = 0;
                u8 r = 0, g = 0, b = 0;
                switch (pixel_format) {
                    case PSP_DISPLAY_PIXEL_FORMAT_565:
                        color = vram16[x + y * buf_width];
                        r = (color & 0x1f) << 3; 
                        g = ((color >> 5) & 0x3f) << 2 ;
                        b = ((color >> 11) & 0x1f) << 3 ;
                        break;
                    
                    case PSP_DISPLAY_PIXEL_FORMAT_5551:
                        color = vram16[x + y * buf_width];
                        r = (color & 0x1f) << 3; 
                        g = ((color >> 5) & 0x1f) << 3 ;
                        b = ((color >> 10) & 0x1f) << 3 ;
                        break;
                        
                    case PSP_DISPLAY_PIXEL_FORMAT_4444:
                        color = vram16[x + y * buf_width];
                        r = (color & 0xf) << 4; 
                        g = ((color >> 4) & 0xf) << 4 ;
                        b = ((color >> 8) & 0xf) << 4 ;
                        break;
                        
                    case PSP_DISPLAY_PIXEL_FORMAT_8888:
                        color = vram32[x + y * buf_width];
                        r = color & 0xff; 
                        g = (color >> 8) & 0xff;
                        b = (color >> 16) & 0xff;
                        break;
                }
                
                line[i++] = r;
                line[i++] = g;
                line[i++] = b;
            }

            png_write_row(png_ptr, line);
        }
        
        delete[] line;
        png_write_end(png_ptr, info_ptr);
        png_destroy_write_struct(&png_ptr, nullptr);
        sceIoClose(file);
    }

    int Capture(void) {
        int ret = 0;
        pspTime time;
        
        if (R_FAILED(ret = sceRtcGetCurrentClockLocalTime(&time)))
            return ret;
            
        if (!(FS::DirExists(Utils::IsInternalStorage()? "ef0:/PSP/PHOTO/CMFileManager/" : "ms0:/PSP/PHOTO/CMFileManager/")))
            FS::RecursiveMakeDir(Utils::IsInternalStorage()? "ef0:/PSP/PHOTO/CMFileManager" : "ms0:/PSP/PHOTO/CMFileManager");
            
        static char path[128];
        std::snprintf(path, 128, Utils::IsInternalStorage()? "ef0:/PSP/PHOTO/CMFileManager/screenshot_%02d%02d%02d-%i.bmp" : 
            "ms0:/PSP/PHOTO/CMFileManager/screenshot_%02d%02d%02d-%02d%02d%02d.png", time.year, time.month, time.day, time.hour, time.minutes, time.seconds);
        
        Screenshot::Save(path);
        return 0;
    }
}
