#include <cstring>

#include "fs.h"
#include "g2d.h"
#include "gui.h"
#include "log.h"
#include "textures.h"
#include "utils.h"

namespace GameLauncher {
    typedef struct {
        u32 magic = 0;
        u32 version = 0;
        u32 sfo_offset = 0;
        u32 icon0_offset = 0;
        u32 icon1_offset = 0;
        u32 pic0_offset = 0;
        u32 pic1_offset = 0;
        u32 snd0_offset = 0;
        u32 psp_offset = 0;
        u32 psar_offset = 0;
    } pbp;
    
    typedef struct {
        u16 key_offset = 0;
        u8 alignment = 0;
        u8 data_type = 0;
        u32 value_size = 0;
        u32 value_size_with_padding = 0;
        u32 data_offset = 0;
    } sfo_index;
    
    typedef struct {
        u32 magic = 0;
        u32 version = 0;
        u32 key_offset = 0;
        u32 value_offset = 0;
        u32 pair_count = 0;
    } sfo;
    
    typedef struct {
        u8 *icon0_data = nullptr;
        int icon0_size = 0;
        u8 *icon1_data = nullptr;
        int icon1_size = 0;
        u8 *pic1_data = nullptr;
        int pic1_size = 0;
        u8 *snd0_data = nullptr;
        int snd0_size = 0;
        char *title = nullptr;
    } eboot_meta;

    int ReadSFOTitle(SceUID file, u8 *buffer, int size, char *id_buf, int id_size) {
        int ret = 0;
        sfo *sfo_data = reinterpret_cast<sfo *>(buffer);
        
        // read the sfo header
        if (R_FAILED(ret = sceIoRead(file, sfo_data, sizeof(sfo)))) {
            Log::Error("GameLauncher::ReadSFOTitle sceIoRead sfo header failed: %08x\n", ret);
            return ret;
        }
        
        // allocate memory to read the sfo block
        u8 *sfo_block = buffer + size;
        if (R_FAILED(ret = sceIoRead(file, sfo_block, size))) {
            Log::Error("GameLauncher::ReadSFOTitle sceIoRead sfo block failed: %08x\n", ret);
            return ret;
        }
        
        // get the sfo index table inside the block
        sfo_index *index_block = reinterpret_cast<sfo_index *>(sfo_block);
        u32 keys_offset_block = sizeof(sfo) + (sizeof(sfo_index) * sfo_data->pair_count);
        u8 *value_block = sfo_block + sfo_data->value_offset - sizeof(sfo);

        for (u32 i = 0; i < sfo_data->pair_count; i++) {
            char *key_addr = reinterpret_cast<char *>(sfo_block) + index_block[i].key_offset + keys_offset_block - sizeof(sfo);
            if (!std::strcmp(key_addr, "TITLE")) {
                std::memcpy(id_buf, value_block, id_size);
                return id_size;
            }
            
            value_block += index_block[i].value_size_with_padding;
        }
        
        return 0;
    }
    
    static int GetMeta(const std::string &path, eboot_meta *meta) {
        int ret = 0;
        char title_buf[128];
        SceUID file = 0;
        pbp pbp_data = {{0}};
        
        if (R_FAILED(ret = file = sceIoOpen(path.c_str(), PSP_O_RDONLY, 0777))) {
            Log::Error("GameLauncher::GetMeta sceIoOpen(%s) failed: %08x\n", path.c_str(), ret);
            return ret;
        }
            
        sceIoRead(file, &pbp_data, sizeof(pbp));
        
        // Get title
        int title_size = pbp_data.icon0_offset - pbp_data.sfo_offset;
        u8 *buffer = new u8[4096];
        meta->title = new char[128];
        GameLauncher::ReadSFOTitle(file, buffer, title_size, title_buf, sizeof(title_buf));
        snprintf(meta->title, 128, title_buf);
        delete[] buffer;
        
        // Get icon0
        sceIoLseek(file, pbp_data.icon0_offset, PSP_SEEK_SET);
        meta->icon0_size = pbp_data.icon1_offset - pbp_data.icon0_offset;
        if (meta->icon0_size) {
            meta->icon0_data = new u8[meta->icon0_size];
            if (R_FAILED(ret = sceIoRead(file, meta->icon0_data, meta->icon0_size))) {
                Log::Error("GameLauncher::GetMeta icon0 sceIoRead(%s) failed: %08x\n", path.c_str(), ret);
                return ret;
            }
        }

        // Get icon1
        sceIoLseek(file, pbp_data.icon1_offset, PSP_SEEK_SET);
        meta->icon1_size = pbp_data.pic0_offset - pbp_data.icon1_offset;
        if (meta->icon1_size) {
            meta->icon1_data = new u8[meta->icon1_size];
            if (R_FAILED(ret = sceIoRead(file, meta->icon1_data, meta->icon1_size))) {
                Log::Error("GameLauncher::GetMeta icon1 sceIoRead(%s) failed: %08x\n", path.c_str(), ret);
                return ret;
            }
        }
        
        // Get pic1
        sceIoLseek(file, pbp_data.pic1_offset, PSP_SEEK_SET);
        meta->pic1_size = pbp_data.snd0_offset - pbp_data.pic1_offset;
        if (meta->pic1_size) {
            meta->pic1_data = new u8[meta->pic1_size];
            if (R_FAILED(ret = sceIoRead(file, meta->pic1_data, meta->pic1_size))) {
                Log::Error("GameLauncher::GetMeta pic1 sceIoRead(%s) failed: %08x\n", path.c_str(), ret);
                return ret;
            }
        }

        // Get snd0
        sceIoLseek(file, pbp_data.snd0_offset, PSP_SEEK_SET);
        meta->snd0_size = pbp_data.psp_offset - pbp_data.snd0_offset;
        if (meta->snd0_size) {
            meta->snd0_data = new u8[meta->snd0_size];
            if (R_FAILED(ret = sceIoRead(file, meta->snd0_data, meta->snd0_size))) {
                Log::Error("GameLauncher::GetMeta snd0 sceIoRead(%s) failed: %08x\n", path.c_str(), ret);
                return ret;
            }
        }
        
        sceIoClose(file);
        return 0;
    }

    int DisplayLauncher(const std::string &path) {
        int ret = 0;
        SceIoStat stat;
        
        if (R_FAILED(ret = sceIoGetstat(path.c_str(), &stat))) {
            Log::Error("DisplayLauncher sceIoGetstat(%s) failed: %08x\n", path.c_str(), ret);
            return ret;
        }

        eboot_meta meta = { 0 };
        if (R_FAILED(ret = GameLauncher::GetMeta(path, &meta)))
            return ret;

        g2dTexture *icon0 = nullptr;
        if (meta.icon0_data)
            icon0 = g2dTexLoadMemory(meta.icon0_data, meta.icon0_size, G2D_SWIZZLE);
        
        char install_date[128];
        const char *months[] = { "Dec", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov" };
        snprintf(install_date, 128, "Installed %d %s %d", stat.st_ctime.day, months[stat.st_ctime.month], stat.st_ctime.year);

        while(true) {
            g2dClear(G2D_RGBA(32, 33, 36, 255));
            G2D::DrawRect(240, 10, 2, 262, G2D_RGBA(78, 80, 85, 255));
            G2D::DrawRect(242, 146, 238, 2, G2D_RGBA(78, 80, 85, 255));
            G2D::DrawImage(ic_play_btn, 312, 125);
            
            if (icon0)
                G2D::DrawImage(icon0, 50, 56);
            else {
                G2D::DrawRect(50, 56, 144, 80, G2D_RGBA(46, 46, 50, 255));
                G2D::FontSetStyle(font, 0.75f, G2D_RGBA(232, 234, 238, 255), INTRAFONT_ALIGN_LEFT);
                G2D::DrawText(50 + ((144 - intraFontMeasureText(font, "ICON0 not found")) / 2), 96, "ICON0 not found");
            }
            
            G2D::DrawRect(260, 190, 200, 50, G2D_RGBA(46, 46, 50, 255));
            
            G2D::FontSetStyle(font, 0.9f, G2D_RGBA(232, 234, 238, 255), INTRAFONT_ALIGN_LEFT);
            G2D::DrawText(242 + ((238 - intraFontMeasureText(font, meta.title)) / 2), 56, meta.title);
            G2D::FontSetStyle(font, 0.75f, G2D_RGBA(232, 234, 238, 255), INTRAFONT_ALIGN_LEFT);
            G2D::DrawText(242 + ((238 - intraFontMeasureText(font, install_date)) / 2), 76, install_date);

            G2D::DrawText(242 + ((238 - intraFontMeasureText(font, "Press Square to save ICON0")) / 2), 210, "Press Square to save ICON0");
            G2D::DrawText(242 + ((238 - intraFontMeasureText(font, "Press Triangle to save PIC1")) / 2), 230, "Press Triangle to save PIC1");
            g2dFlip(G2D_VSYNC);

            Utils::ReadControls();
            
            if (Utils::IsButtonPressed(PSP_CTRL_ENTER))
                Utils::LaunchEboot(path.c_str());
                
            if (Utils::IsButtonPressed(PSP_CTRL_SQUARE)) {
                if (meta.icon0_data) {
                    std::string icon0_path = Utils::IsInternalStorage()? "ef0:" : "ms0:";
                    icon0_path.append("/PSP/PHOTO/CMFileManager/");
                    icon0_path.append(meta.title);
                    
                    if (!(FS::DirExists(icon0_path)))
                        FS::RecursiveMakeDir(icon0_path);
                    
                    icon0_path.append("/icon0.png");
                    FS::WriteFile(icon0_path, meta.icon0_data, meta.icon0_size);
                }
            }
            
            if (Utils::IsButtonPressed(PSP_CTRL_TRIANGLE)) {
                if (meta.pic1_data) {
                    std::string pic1_path = Utils::IsInternalStorage()? "ef0:" : "ms0:";
                    pic1_path.append("/PSP/PHOTO/CMFileManager/");
                    pic1_path.append(meta.title);
                    
                    if (!(FS::DirExists(pic1_path)))
                        FS::RecursiveMakeDir(pic1_path);
                    
                    pic1_path.append("/pic1.png");
                    FS::WriteFile(pic1_path, meta.pic1_data, meta.pic1_size);
                }
            }
            
            if (Utils::IsButtonPressed(PSP_CTRL_CANCEL))
                break;
        }
        
        if (icon0)
            g2dTexFree(&icon0);
            
        if (meta.title)
            delete[] meta.title;
            
        if (meta.icon0_data)
            delete[] meta.icon0_data;

        return 0;
    }
}
