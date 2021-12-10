#include <assert.h>
#include <cstring>

#include "fs.h"
#include "libnsbmp.h"
#include "libnsgif.h"
#include "libpng/png.h"
#include "log.h"
#include "textures.h"
#include "turbojpeg/turbojpeg.h"
#include "utils.h"

#define BYTES_PER_PIXEL 4
#define MAX_IMAGE_BYTES (48 * 1024 * 1024)

extern unsigned char ic_fso_type_app_png_start[], ic_fso_type_compress_png_start[], ic_fso_type_audio_png_start[], 
    ic_fso_folder_png_start[], ic_fso_folder_dark_png_start[], ic_fso_default_png_start[], ic_fso_type_image_png_start[], 
    ic_fso_type_text_png_start[], btn_material_light_check_on_normal_png_start[], btn_material_light_check_on_normal_dark_png_start[], 
    btn_material_light_check_off_normal_png_start[], btn_material_light_check_off_normal_dark_png_start[], btn_material_light_toggle_on_normal_png_start[], 
    btn_material_light_toggle_on_normal_dark_png_start[], btn_material_light_toggle_off_normal_png_start[], btn_material_light_radio_off_normal_png_start[], 
    btn_material_light_radio_on_normal_png_start[], btn_material_light_radio_off_normal_dark_png_start[], btn_material_light_radio_on_normal_dark_png_start[], 
    ic_material_light_navigation_drawer_png_start[], ic_arrow_back_normal_png_start[], ic_material_options_dialog_png_start[], 
    ic_material_options_dialog_dark_png_start[], ic_material_properties_dialog_png_start[], ic_material_properties_dialog_dark_png_start[], 
    ic_material_dialog_png_start[], ic_material_dialog_dark_png_start[], battery_20_png_start[], battery_20_charging_png_start[], battery_30_png_start[], 
    battery_30_charging_png_start[], battery_50_png_start[], battery_50_charging_png_start[], battery_60_png_start[], battery_60_charging_png_start[], 
    battery_80_png_start[], battery_80_charging_png_start[], battery_full_png_start[], battery_full_charging_png_start[], ic_material_light_usb_png_start[], 
    default_artwork_png_start[], default_artwork_blur_png_start[], btn_playback_play_png_start[], btn_playback_pause_png_start[], 
    btn_playback_rewind_png_start[], btn_playback_forward_png_start[], btn_playback_repeat_png_start[], btn_playback_shuffle_png_start[], 
    btn_playback_repeat_overlay_png_start[], btn_playback_shuffle_overlay_png_start[], bg_header_png_start[], ic_material_light_sdcard_png_start[], 
    ic_material_light_secure_png_start[], ic_material_light_sdcard_dark_png_start[], ic_material_light_secure_dark_png_start[], ic_play_btn_png_start[], 
    ftp_icon_png_start[], sort_icon_png_start[], dark_theme_icon_png_start[], dev_options_icon_png_start[], about_icon_png_start[], 
    ftp_icon_dark_png_start[], sort_icon_dark_png_start[], dark_theme_icon_dark_png_start[], dev_options_icon_dark_png_start[], about_icon_dark_png_start[];

extern unsigned int ic_fso_type_app_png_size, ic_fso_type_compress_png_size, ic_fso_type_audio_png_size, ic_fso_folder_png_size, 
    ic_fso_folder_dark_png_size, ic_fso_default_png_size, ic_fso_type_image_png_size, ic_fso_type_text_png_size, 
    btn_material_light_check_on_normal_png_size, btn_material_light_check_on_normal_dark_png_size, btn_material_light_check_off_normal_png_size, 
    btn_material_light_check_off_normal_dark_png_size, btn_material_light_toggle_on_normal_png_size, btn_material_light_toggle_on_normal_dark_png_size, 
    btn_material_light_toggle_off_normal_png_size, btn_material_light_radio_off_normal_png_size, btn_material_light_radio_on_normal_png_size, 
    btn_material_light_radio_off_normal_dark_png_size, btn_material_light_radio_on_normal_dark_png_size, ic_material_light_navigation_drawer_png_size, 
    ic_arrow_back_normal_png_size, ic_material_options_dialog_png_size, ic_material_options_dialog_dark_png_size, ic_material_properties_dialog_png_size, 
    ic_material_properties_dialog_dark_png_size, ic_material_dialog_png_size, ic_material_dialog_dark_png_size, battery_20_png_size, 
    battery_20_charging_png_size, battery_30_png_size, battery_30_charging_png_size, battery_50_png_size, battery_50_charging_png_size, 
    battery_60_png_size, battery_60_charging_png_size, battery_80_png_size, battery_80_charging_png_size, battery_full_png_size, 
    battery_full_charging_png_size, ic_material_light_usb_png_size, 
    default_artwork_png_size, default_artwork_blur_png_size, btn_playback_play_png_size, btn_playback_pause_png_size, btn_playback_rewind_png_size, 
    btn_playback_forward_png_size, btn_playback_repeat_png_size, btn_playback_shuffle_png_size, btn_playback_repeat_overlay_png_size, 
    btn_playback_shuffle_overlay_png_size, bg_header_png_size, ic_material_light_sdcard_png_size, ic_material_light_secure_png_size, 
    ic_material_light_sdcard_dark_png_size, ic_material_light_secure_dark_png_size, ic_play_btn_png_size, 
    ftp_icon_png_size, sort_icon_png_size, dark_theme_icon_png_size, dev_options_icon_png_size, about_icon_png_size, 
    ftp_icon_dark_png_size, sort_icon_dark_png_size, dark_theme_icon_dark_png_size, dev_options_icon_dark_png_size, about_icon_dark_png_size;

g2dTexture *file_icons[NUM_FILE_ICONS], *icon_dir[NUM_THEMES], *icon_check[NUM_THEMES], *icon_uncheck[NUM_THEMES], \
    *icon_toggle_on[NUM_THEMES], *icon_toggle_off, *icon_radio_off[NUM_THEMES], *icon_radio_on[NUM_THEMES], *icon_nav_drawer, \
    *icon_back, *options_dialog[NUM_THEMES], *properties_dialog[NUM_THEMES], *dialog[NUM_THEMES], \
    *battery_charging[NUM_BATT_ICONS], *battery[NUM_BATT_ICONS], *usb_icon, \
    *default_artwork, *default_artwork_blur, *btn_play, *btn_pause, *btn_rewind, *btn_forward, \
    *btn_repeat, *btn_shuffle, *btn_repeat_overlay, *btn_shuffle_overlay, \
    *bg_header, *icon_sd[NUM_THEMES], *icon_secure[NUM_THEMES], *ic_play_btn, *ftp_icon[NUM_THEMES], *sort_icon[NUM_THEMES], \
    *dark_theme_icon[NUM_THEMES], *dev_options_icon[NUM_THEMES], *about_icon[NUM_THEMES];

namespace BMP {
    static void *bitmap_create(int width, int height, [[maybe_unused]] unsigned int state) {
        /* ensure a stupidly large (>50Megs or so) bitmap is not created */
        if ((static_cast<long long>(width) * static_cast<long long>(height)) > (MAX_IMAGE_BYTES/BYTES_PER_PIXEL))
            return nullptr;
        
        return std::calloc(width * height, BYTES_PER_PIXEL);
    }
    
    static unsigned char *bitmap_get_buffer(void *bitmap) {
        assert(bitmap);
        return static_cast<unsigned char *>(bitmap);
    }
    
    static size_t bitmap_get_bpp([[maybe_unused]] void *bitmap) {
        return BYTES_PER_PIXEL;
    }
    
    static void bitmap_destroy(void *bitmap) {
        assert(bitmap);
        std::free(bitmap);
    }
}

namespace GIF {
    static void *bitmap_create(int width, int height) {
        /* ensure a stupidly large bitmap is not created */
        if ((static_cast<long long>(width) * static_cast<long long>(height)) > (MAX_IMAGE_BYTES/BYTES_PER_PIXEL))
            return nullptr;
        
        return std::calloc(width * height, BYTES_PER_PIXEL);
    }

    static void bitmap_set_opaque([[maybe_unused]] void *bitmap, [[maybe_unused]] bool opaque) {
        assert(bitmap);
    }
    
    static bool bitmap_test_opaque([[maybe_unused]] void *bitmap) {
        assert(bitmap);
        return false;
    }
    
    static unsigned char *bitmap_get_buffer(void *bitmap) {
        assert(bitmap);
        return static_cast<unsigned char *>(bitmap);
    }
    
    static void bitmap_destroy(void *bitmap) {
        assert(bitmap);
        std::free(bitmap);
    }
    
    static void bitmap_modified([[maybe_unused]] void *bitmap) {
        assert(bitmap);
        return;
    }
}

namespace Textures {
    static g2dTexture *LoadImageBufferBMP(unsigned char *data, int size) {
        bmp_bitmap_callback_vt bitmap_callbacks = {
            BMP::bitmap_create,
            BMP::bitmap_destroy,
            BMP::bitmap_get_buffer,
            BMP::bitmap_get_bpp
        };
        
        bmp_result code = BMP_OK;
        bmp_image bmp;
        bmp_create(&bmp, &bitmap_callbacks);
            
        code = bmp_analyse(&bmp, size, data);
        if (code != BMP_OK) {
            Log::Error("bmp_analyse failed: %d\n", code);
            bmp_finalise(&bmp);
            return nullptr;
        }

        code = bmp_decode(&bmp);
        if (code != BMP_OK) {
            if ((code != BMP_INSUFFICIENT_DATA) && (code != BMP_DATA_ERROR)) {
                Log::Error("bmp_decode failed: %d\n", code);
                bmp_finalise(&bmp);
                return nullptr;
            }
            
            /* skip if the decoded image would be ridiculously large */
            if ((bmp.width * bmp.height) > 200000) {
                Log::Error("bmp_decode failed: width*height is over 200000\n");
                bmp_finalise(&bmp);
                return nullptr;
            }
        }
        
        g2dTexture *tex = g2dTexLoad(static_cast<unsigned char *>(bmp.bitmap), bmp.width, bmp.height, G2D_SWIZZLE);
        bmp_finalise(&bmp);
        return tex;
    }

    static g2dTexture *LoadImageBufferGIF(unsigned char *data, int size) {
        gif_bitmap_callback_vt bitmap_callbacks = {
            GIF::bitmap_create,
            GIF::bitmap_destroy,
            GIF::bitmap_get_buffer,
            GIF::bitmap_set_opaque,
            GIF::bitmap_test_opaque,
            GIF::bitmap_modified
        };
        
        gif_animation gif;
        gif_result code = GIF_OK;
        gif_create(&gif, &bitmap_callbacks);
            
        do {
            code = gif_initialise(&gif, size, data);
            if (code != GIF_OK && code != GIF_WORKING) {
                Log::Error("gif_initialise failed: %d\n", code);
                gif_finalise(&gif);
                return nullptr;
            }
        } while (code != GIF_OK);
        
        code = gif_decode_frame(&gif, 0);
        if (code != GIF_OK) {
            Log::Error("gif_decode_frame failed: %d\n", code);
            return nullptr;
        }
        
        g2dTexture *tex = g2dTexLoad(static_cast<unsigned char *>(gif.frame_image), gif.width, gif.height, G2D_SWIZZLE);
        gif_finalise(&gif);
        return tex;
    }

    g2dTexture *LoadImageBufferJPEG(unsigned char *data, int size) {
        int width = 0; int height = 0;
        unsigned char *buffer = nullptr;
        tjhandle jpeg = tjInitDecompress();
        int jpegsubsamp = 0;

        if (R_FAILED(tjDecompressHeader2(jpeg, data, size, &width, &height, &jpegsubsamp))) {
            Log::Error("tjDecompressHeader2 failed: %s\n", tjGetErrorStr());
            delete[] data;
            return nullptr;
        }
        
        buffer = new unsigned char[width * height * 4];

        if (R_FAILED(tjDecompress2(jpeg, data, size, buffer, width, 0, height, TJPF_RGBA, TJFLAG_FASTDCT))) {
            Log::Error("tjDecompress2 failed: %s\n", tjGetErrorStr());
            delete[] buffer;
            return nullptr;
        }
        
        g2dTexture *tex = g2dTexLoad(buffer, width, height, G2D_SWIZZLE);
        tjDestroy(jpeg);
        delete[] buffer;
        return tex;
    }

    g2dTexture *LoadImageBufferPNG(unsigned char *data, int size) {
        g2dTexture *tex = nullptr;
        png_image image;
        std::memset(&image, 0, (sizeof image));
        image.version = PNG_IMAGE_VERSION;
        
        if (png_image_begin_read_from_memory(&image, data, size) != 0) {
            png_bytep buffer;
            image.format = PNG_FORMAT_RGBA;
            buffer = new png_byte[PNG_IMAGE_SIZE(image)];
            
            if (buffer != nullptr && png_image_finish_read(&image, nullptr, buffer, 0, nullptr) != 0) {
                tex = g2dTexLoad(buffer, image.width, image.height, G2D_SWIZZLE);
                delete[] buffer;
                png_image_free(&image);
            }
            else {
                if (buffer == nullptr) {
                    Log::Error("png_byte buffer: returned nullptr\n");
                    png_image_free(&image);
                }
                else {
                    Log::Error("png_image_finish_read failed: %s\n", image.message);
                    delete[] buffer;
                }
            }
        }
        else
            Log::Error("png_image_begin_read_from_memory failed: %s\n", image.message);
        
        return tex;
    }

    g2dTexture *LoadImage(const std::string &path) {
        int ret = 0;
        u64 size = FS::GetFileSize(path);
        unsigned char *data = new unsigned char[size];

        if (R_FAILED(ret = FS::ReadFile(path, data, size))) {
            Log::Error("LoadImage FS::ReadFile failed 0x%08x\n", ret);
            delete[] data;
            return nullptr;
        }

        g2dTexture *tex = nullptr;
        std::string ext = FS::GetFileExt(path);

        if (ext == ".BMP")
            tex = Textures::LoadImageBufferBMP(data, size);
        else if (ext == ".GIF")
            tex = Textures::LoadImageBufferGIF(data, size);
        else if ((ext == ".JPEG") || (ext == ".JPG"))
            tex = Textures::LoadImageBufferJPEG(data, size);
        else if (ext == ".PNG")
            tex = Textures::LoadImageBufferPNG(data, size);
        
        delete[] data;
        return tex;
    }

    void Load(void) {
        file_icons[0] = Textures::LoadImageBufferPNG(ic_fso_default_png_start, ic_fso_default_png_size);
        file_icons[1] = Textures::LoadImageBufferPNG(ic_fso_type_app_png_start, ic_fso_type_app_png_size);
        file_icons[2] = Textures::LoadImageBufferPNG(ic_fso_type_compress_png_start, ic_fso_type_compress_png_size);
        file_icons[3] = Textures::LoadImageBufferPNG(ic_fso_type_audio_png_start, ic_fso_type_audio_png_size);
        file_icons[4] = Textures::LoadImageBufferPNG(ic_fso_type_image_png_start, ic_fso_type_image_png_size);
        file_icons[5] = Textures::LoadImageBufferPNG(ic_fso_type_text_png_start, ic_fso_type_text_png_size);

        icon_dir[0] = Textures::LoadImageBufferPNG(ic_fso_folder_png_start, ic_fso_folder_png_size);
        icon_dir[1] = Textures::LoadImageBufferPNG(ic_fso_folder_dark_png_start, ic_fso_folder_dark_png_size);
        icon_check[0] = Textures::LoadImageBufferPNG(btn_material_light_check_on_normal_png_start, btn_material_light_check_on_normal_png_size);
        icon_check[1] = Textures::LoadImageBufferPNG(btn_material_light_check_on_normal_dark_png_start, btn_material_light_check_on_normal_dark_png_size);
        icon_uncheck[0] = Textures::LoadImageBufferPNG(btn_material_light_check_off_normal_png_start, btn_material_light_check_off_normal_png_size);
        icon_uncheck[1] = Textures::LoadImageBufferPNG(btn_material_light_check_off_normal_dark_png_start, btn_material_light_check_off_normal_dark_png_size);
        icon_toggle_on[0] = Textures::LoadImageBufferPNG(btn_material_light_toggle_on_normal_png_start, btn_material_light_toggle_on_normal_png_size);
        icon_toggle_on[1] = Textures::LoadImageBufferPNG(btn_material_light_toggle_on_normal_dark_png_start, btn_material_light_toggle_on_normal_dark_png_size);
        icon_toggle_off = Textures::LoadImageBufferPNG(btn_material_light_toggle_off_normal_png_start, btn_material_light_toggle_off_normal_png_size);
        icon_radio_off[0] = Textures::LoadImageBufferPNG(btn_material_light_radio_off_normal_png_start, btn_material_light_radio_off_normal_png_size);
        icon_radio_off[1] = Textures::LoadImageBufferPNG(btn_material_light_radio_off_normal_dark_png_start, btn_material_light_radio_off_normal_dark_png_size);
        icon_radio_on[0] = Textures::LoadImageBufferPNG(btn_material_light_radio_on_normal_png_start, btn_material_light_radio_on_normal_png_size);
        icon_radio_on[1] = Textures::LoadImageBufferPNG(btn_material_light_radio_on_normal_dark_png_start, btn_material_light_radio_on_normal_dark_png_size);
        icon_nav_drawer = Textures::LoadImageBufferPNG(ic_material_light_navigation_drawer_png_start, ic_material_light_navigation_drawer_png_size);
        icon_back = Textures::LoadImageBufferPNG(ic_arrow_back_normal_png_start, ic_arrow_back_normal_png_size);
        options_dialog[0] = Textures::LoadImageBufferPNG(ic_material_options_dialog_png_start, ic_material_options_dialog_png_size);
        options_dialog[1] = Textures::LoadImageBufferPNG(ic_material_options_dialog_dark_png_start, ic_material_options_dialog_dark_png_size);
        properties_dialog[0] = Textures::LoadImageBufferPNG(ic_material_properties_dialog_png_start, ic_material_properties_dialog_png_size);
        properties_dialog[1] = Textures::LoadImageBufferPNG(ic_material_properties_dialog_dark_png_start, ic_material_properties_dialog_dark_png_size);
        dialog[0] = Textures::LoadImageBufferPNG(ic_material_dialog_png_start, ic_material_dialog_png_size);
        dialog[1] = Textures::LoadImageBufferPNG(ic_material_dialog_dark_png_start, ic_material_dialog_dark_png_size);

        battery_charging[0] = Textures::LoadImageBufferPNG(battery_20_charging_png_start, battery_20_charging_png_size);
        battery_charging[1] = Textures::LoadImageBufferPNG(battery_30_charging_png_start, battery_30_charging_png_size);
        battery_charging[2] = Textures::LoadImageBufferPNG(battery_50_charging_png_start, battery_50_charging_png_size);
        battery_charging[3] = Textures::LoadImageBufferPNG(battery_60_charging_png_start, battery_60_charging_png_size);
        battery_charging[4] = Textures::LoadImageBufferPNG(battery_80_charging_png_start, battery_80_charging_png_size);
        battery_charging[5] = Textures::LoadImageBufferPNG(battery_full_charging_png_start, battery_full_charging_png_size);
        
        battery[0] = Textures::LoadImageBufferPNG(battery_20_png_start, battery_30_png_size);
        battery[1] = Textures::LoadImageBufferPNG(battery_30_png_start, battery_30_png_size);
        battery[2] = Textures::LoadImageBufferPNG(battery_50_png_start, battery_50_png_size);
        battery[3] = Textures::LoadImageBufferPNG(battery_60_png_start, battery_60_png_size);
        battery[4] = Textures::LoadImageBufferPNG(battery_80_png_start, battery_80_png_size);
        battery[5] = Textures::LoadImageBufferPNG(battery_full_png_start, battery_full_png_size);
        
        usb_icon = Textures::LoadImageBufferPNG(ic_material_light_usb_png_start, ic_material_light_usb_png_size);
        
        default_artwork = Textures::LoadImageBufferPNG(default_artwork_png_start, default_artwork_png_size);
        default_artwork_blur = Textures::LoadImageBufferPNG(default_artwork_blur_png_start, default_artwork_blur_png_size);
        btn_play = Textures::LoadImageBufferPNG(btn_playback_play_png_start, btn_playback_play_png_size);
        btn_pause = Textures::LoadImageBufferPNG(btn_playback_pause_png_start, btn_playback_pause_png_size);
        btn_rewind = Textures::LoadImageBufferPNG(btn_playback_rewind_png_start, btn_playback_rewind_png_size);
        btn_forward = Textures::LoadImageBufferPNG(btn_playback_forward_png_start, btn_playback_forward_png_size);
        btn_repeat = Textures::LoadImageBufferPNG(btn_playback_repeat_png_start, btn_playback_repeat_png_size);
        btn_shuffle = Textures::LoadImageBufferPNG(btn_playback_shuffle_png_start, btn_playback_shuffle_png_size);
        btn_repeat_overlay = Textures::LoadImageBufferPNG(btn_playback_repeat_overlay_png_start, btn_playback_repeat_overlay_png_size);
        btn_shuffle_overlay = Textures::LoadImageBufferPNG(btn_playback_shuffle_overlay_png_start, btn_playback_shuffle_overlay_png_size);
        
        bg_header = Textures::LoadImageBufferPNG(bg_header_png_start, bg_header_png_size);
        icon_sd[0] = Textures::LoadImageBufferPNG(ic_material_light_sdcard_png_start, ic_material_light_sdcard_png_size);
        icon_sd[1] = Textures::LoadImageBufferPNG(ic_material_light_sdcard_dark_png_start, ic_material_light_sdcard_dark_png_size);
        icon_secure[0] = Textures::LoadImageBufferPNG(ic_material_light_secure_png_start, ic_material_light_secure_png_size);
        icon_secure[1] = Textures::LoadImageBufferPNG(ic_material_light_secure_dark_png_start, ic_material_light_secure_dark_png_size);
        ic_play_btn = Textures::LoadImageBufferPNG(ic_play_btn_png_start, ic_play_btn_png_size);

        ftp_icon[0] = Textures::LoadImageBufferPNG(ftp_icon_png_start, ftp_icon_png_size);
        ftp_icon[1] = Textures::LoadImageBufferPNG(ftp_icon_dark_png_start, ftp_icon_dark_png_size);
        sort_icon[0] = Textures::LoadImageBufferPNG(sort_icon_png_start, sort_icon_png_size);
        sort_icon[1] = Textures::LoadImageBufferPNG(sort_icon_dark_png_start, sort_icon_dark_png_size);
        dark_theme_icon[0] = Textures::LoadImageBufferPNG(dark_theme_icon_png_start, dark_theme_icon_png_size);
        dark_theme_icon[1] = Textures::LoadImageBufferPNG(dark_theme_icon_dark_png_start, dark_theme_icon_dark_png_size);
        dev_options_icon[0] = Textures::LoadImageBufferPNG(dev_options_icon_png_start, dev_options_icon_png_size);
        dev_options_icon[1] = Textures::LoadImageBufferPNG(dev_options_icon_dark_png_start, dev_options_icon_dark_png_size);
        about_icon[0] = Textures::LoadImageBufferPNG(about_icon_png_start, about_icon_png_size);
        about_icon[1] = Textures::LoadImageBufferPNG(about_icon_dark_png_start, about_icon_dark_png_size);
    }

    void Free(void) {
        g2dTexFree(&ic_play_btn);
        g2dTexFree(&bg_header);
        g2dTexFree(&btn_shuffle_overlay);
        g2dTexFree(&btn_repeat_overlay);
        g2dTexFree(&btn_shuffle);
        g2dTexFree(&btn_repeat);
        g2dTexFree(&btn_forward);
        g2dTexFree(&btn_rewind);
        g2dTexFree(&btn_pause);
        g2dTexFree(&btn_play);
        g2dTexFree(&default_artwork_blur);
        g2dTexFree(&default_artwork);
        g2dTexFree(&usb_icon);
        g2dTexFree(&icon_back);
        g2dTexFree(&icon_nav_drawer);
        g2dTexFree(&icon_toggle_off);

        for (int i = 0; i < NUM_THEMES; i++) {
            g2dTexFree(&about_icon[i]);
            g2dTexFree(&dev_options_icon[i]);
            g2dTexFree(&sort_icon[i]);
            g2dTexFree(&ftp_icon[i]);
            g2dTexFree(&icon_secure[i]);
            g2dTexFree(&icon_sd[i]);
            g2dTexFree(&dialog[i]);
            g2dTexFree(&properties_dialog[i]);
            g2dTexFree(&options_dialog[i]);
            g2dTexFree(&icon_radio_on[i]);
            g2dTexFree(&icon_radio_off[i]);
            g2dTexFree(&icon_toggle_on[i]);
            g2dTexFree(&icon_uncheck[i]);
            g2dTexFree(&icon_check[i]);
            g2dTexFree(&icon_dir[i]);
        }
        
        for (int i = 0; i < NUM_FILE_ICONS; i++)
            g2dTexFree(&file_icons[i]);
            
        for (int i = 0; i < NUM_BATT_ICONS; i++) {
            g2dTexFree(&battery_charging[i]);
            g2dTexFree(&battery[i]);
        }
    }
}
