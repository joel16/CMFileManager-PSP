#include <cstring>
#include <zlib.h>

#include "fs.h"
#include "g2d.h"
#include "gui.h"
#include "log.h"
#include "textures.h"
#include "utils.h"

// Heavily based on TN Menu's ISO reader with some c++ code style changes, casts, and modified to get icon1, pic1 and snd data.

#define BINARY2INT(var, addr) {                           \
        *(((char *)(var)) + 0) = *(((char *)(addr)) + 0); \
        *(((char *)(var)) + 1) = *(((char *)(addr)) + 1); \
        *(((char *)(var)) + 2) = *(((char *)(addr)) + 2); \
        *(((char *)(var)) + 3) = *(((char *)(addr)) + 3); \
    }

namespace CSO {
    typedef struct {
        u8 magic[4] = {0};                  /* +00 : 'C','I','S','O'                 */
        unsigned long header_size = 0;      /* +04 : header size (==0x18)            */
        unsigned long long total_bytes = 0; /* +08 : number of original data size    */
        unsigned long block_size = 0;       /* +10 : number of compressed block size */
        u8 ver = 0;                         /* +14 : version 01                      */
        u8 align = 0;                       /* +15 : align of index value            */
        u8 rsv_06[2] = {0};                 /* +16 : reserved                        */
    } CISOHeader;

    constexpr SceOff SECTOR_SIZE = 0x800;

    int Inflate(char *o_buff, int o_size, char *i_buff, int i_size) {
        z_stream z;
        int size = 0;
        
        z.zalloc = Z_NULL;
        z.zfree = Z_NULL;
        z.opaque = Z_NULL;
        z.next_in = Z_NULL;
        z.avail_in = 0;
        
        if (inflateInit2(&z, -15) != Z_OK)
            return -1;
            
        z.next_in = reinterpret_cast<u8 *>(i_buff);
        z.avail_in = i_size;
        z.next_out = reinterpret_cast<u8 *>(o_buff);
        z.avail_out = o_size;
        
        inflate(&z, Z_FINISH);
        size = o_size - z.avail_out;
        if (inflateEnd(&z) != Z_OK)
            return -1;
            
        return size;
    }
    
    int ReadFile(char *buf, SceUID fd, int pos, int size) {
        static CISOHeader ciso;
        int index = 0, index2 = 0;
        char tmp_buf[SECTOR_SIZE * 2] = {0}, tmp_buf_2[SECTOR_SIZE * 2] = {0};
        int ret = 0, error = 0;
        
        if (R_FAILED(error = sceIoLseek(fd, 0, PSP_SEEK_SET)))
            return error;
            
        if (R_FAILED(error = sceIoRead(fd, &ciso, sizeof(ciso))))
            return error;
            
        if (static_cast<unsigned long long>((pos + size)) > ciso.total_bytes)
            size = ciso.total_bytes - pos;
            
        int max_sector = ciso.total_bytes / ciso.block_size - 1;
        int start_sec = pos / SECTOR_SIZE;
        int end_sec = (pos + size - 1) / SECTOR_SIZE;
        int sector_num = start_sec;
        
        if (sector_num > max_sector)
            return -1;
            
        if (end_sec > max_sector)
            end_sec = max_sector;
            
        ret = 0;
        while(sector_num <= end_sec) {
            if (R_FAILED(error = sceIoLseek(fd, sizeof(ciso) + (sector_num * 4), PSP_SEEK_SET)))
                return error;

            if (R_FAILED(error = sceIoRead(fd, &index, 4)))
                return error;
                
            u32 zip_flag = index & 0x80000000;
            index = (index & 0x7FFFFFFF) << ciso.align;

            if (R_FAILED(error = sceIoRead(fd, &index2, 4)))
                return error;
                
            int read_size = ((index2 & 0x7FFFFFFF) << ciso.align) - index;

            if (R_FAILED(error = sceIoLseek(fd, index, PSP_SEEK_SET)))
                return error;
                
            if (zip_flag != 0) {
                if (R_FAILED(error = sceIoRead(fd, tmp_buf, ciso.block_size)))
                    return error;
            }
            else {
                if (R_FAILED(error = sceIoRead(fd, tmp_buf_2, read_size)))
                    return error;

                if (R_FAILED(error = CSO::Inflate(tmp_buf, ciso.block_size, tmp_buf_2, read_size)))
                    return error;
            }
            
            if ((sector_num > start_sec) && (sector_num < end_sec)) {
                std::memcpy(buf, tmp_buf, ciso.block_size);
                read_size = ciso.block_size;
            }
            else if ((sector_num == start_sec) || (sector_num == end_sec)) {
                int start_pos = 0;
                int end_pos = ciso.block_size;
                
                if (sector_num == start_sec)
                    start_pos = pos - (start_sec * ciso.block_size);
                
                if (sector_num == end_sec)
                    end_pos = (pos + size) - (end_sec * ciso.block_size);
                    
                read_size = end_pos - start_pos;
                std::memcpy(buf, &tmp_buf[start_pos], read_size);
            }
            
            buf += read_size;
            ret += read_size;
            sector_num++;
        }
        
        return ret;
    }
    
    int Read(char *buf, const std::string &path, int pos, int size) {
        SceUID file = 0;

        if (R_FAILED(file = sceIoOpen(path.c_str(), PSP_O_RDONLY, 0))) {
            Log::Error("CSO::Read sceIoOpen(%s) failed: 0x%08x\n", path.c_str(), file);
            return file;
        }
            
        int ret = CSO::ReadFile(buf, file, pos, size);
        sceIoClose(file);
        return ret;
    }
}

namespace ISO {
    constexpr SceOff SECTOR_SIZE = 0x800;

    SceUID Read(void *buf, const std::string &path, int pos, int size) {
        SceUID file = 0;

        if (R_FAILED(file = sceIoOpen(path.c_str(), PSP_O_RDONLY, 0))) {
            Log::Error("ISO::Read sceIoOpen(%s) failed: 0x%08x\n", path.c_str(), file);
            return file;
        }
            
        sceIoLseek(file, pos, PSP_SEEK_SET);
        int read = sceIoRead(file, buf, size);
        sceIoClose(file);
        return read;
    }
    
    int ReadFile(void *buf, const std::string &path, int type, int pos, int size) {
        if (type == 0)
            return ISO::Read(buf, path, pos, size);
        else if (type == 1)
            return CSO::Read(reinterpret_cast<char *>(buf), path, pos, size);
            
        return -1;
    }
    
    int GetInfo(int *pos, int *size, int *size_pos, const std::string &path, int type, const char *name) {
        char work[256] = {0}, s_path[256] = {0}, s_file[256] = {0};
        int path_table_addr = 0, path_table_size = 0;
        int dir_recode_addr = 0;
        short int befor_dir_num = 0x0001;
        int now_dir_num = 1;
        int ret = 0;

        if (*name == '/')
            name++;
            
        std::strcpy(work, name);
        char *ptr = std::strrchr(work, '/');
        
        if (ptr != nullptr) {
            *ptr++ = '\0';
            std::strcpy(s_path, work);
        }
        else {
            s_path[0] = '\0';
            ptr = static_cast<char *>(work);
        }
        
        std::strcpy(s_file, ptr);
        
        ///////
        u8 header[8] = {0};
        ISO::ReadFile(header, path, type, 0x8000, sizeof(header));
        u8 magic[8] = { 0x01, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01, 0x00 };
        
        //Invalid Magic
        if (std::memcmp(header, magic, sizeof(header)) != 0)
            return -1;
        ///////
        
        ISO::ReadFile(&path_table_size, path, type, 0x8084, 4);
        ISO::ReadFile(&path_table_addr, path, type, 0x808C, 4);
        path_table_addr *= SECTOR_SIZE;
        
        char *table_buf = new char[path_table_size];
        if (!table_buf)
            return -1;
        
        ret = ISO::ReadFile(table_buf, path, type, path_table_addr, path_table_size);
        
        if (s_path[0] == '\0') {
            BINARY2INT(&dir_recode_addr, &table_buf[2]);
        }
        else {
            befor_dir_num = 0x0001;
            
            int tbl_ptr = 0;
            now_dir_num = 0x0001;
            ptr = s_path;
            
            while(tbl_ptr < path_table_size) {
                u8 len_di = static_cast<u8>(table_buf[tbl_ptr]);
                if (len_di == 0)
                    break;
                    
                tbl_ptr += 6;
                
                if (befor_dir_num == *reinterpret_cast<short int *>(&table_buf[tbl_ptr])) {
                    tbl_ptr += 2;
                    
                    if (strncasecmp(&table_buf[tbl_ptr], ptr, len_di) == 0) {
                        befor_dir_num = now_dir_num;
                        ptr = strchr(ptr, '/');
                        
                        if (ptr != nullptr)
                            ptr++;
                        else {
                            BINARY2INT(&dir_recode_addr, &table_buf[tbl_ptr - 6]);
                            break;
                        }
                    }
                }
                else
                    tbl_ptr += 2;
                    
                tbl_ptr += (len_di + 1) & ~1; // padding
                now_dir_num++;
            }
        }
        
        delete[] table_buf;
        
        if (dir_recode_addr == 0)
            return -1;
            
        dir_recode_addr *= SECTOR_SIZE;
        int dir_record_size = 0;
        ret = ISO::ReadFile(&dir_record_size, path, type, dir_recode_addr + 10, 4);
        char *dir_buf = new char[dir_record_size];
        if (!dir_buf)
            return -1;
            
        int dir_ptr = 0;
        ret = -1;
        ISO::ReadFile(dir_buf, path, type, dir_recode_addr, dir_record_size);
        
        while(dir_ptr < dir_record_size) {
            u8 len_dr = static_cast<u8>(dir_buf[dir_ptr]);
            if (len_dr == 0)
                dir_ptr++;
            else {
                if (strncasecmp(&dir_buf[dir_ptr + 33], s_file, dir_buf[dir_ptr + 32]) == 0) {
                    BINARY2INT(pos, &dir_buf[dir_ptr + 2]);
                    *pos *= SECTOR_SIZE;
                    BINARY2INT(size, &dir_buf[dir_ptr + 10]);
                    *size_pos = dir_recode_addr + dir_ptr + 10;
                    ret = 0;
                    break;
                }
                
                dir_ptr += len_dr;
            }
        }
        
        delete[] dir_buf;
        return ret;
    }
}

namespace GameLauncher {
    typedef struct {
        int pos = 0;
        int size = 0;
        int size_pos = 0;
        int dir_tbl_pos = 0;
        int path_tbl_pos = 0;
    } ISOData;

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
    } PBPHeader;
    
    typedef struct {
        u16 key_offset = 0;
        u8 alignment = 0;
        u8 data_type = 0;
        u32 value_size = 0;
        u32 value_size_with_padding = 0;
        u32 data_offset = 0;
    } SFOIndex;
    
    typedef struct {
        u32 magic = 0;
        u32 version = 0;
        u32 key_offset = 0;
        u32 value_offset = 0;
        u32 pair_count = 0;
    } SFO;
    
    typedef struct {
        u8 *icon0_data = nullptr;
        int icon0_size = 0;
        u8 *icon1_data = nullptr;
        int icon1_size = 0;
        u8 *pic0_data = nullptr;
        int pic0_size = 0;
        u8 *pic1_data = nullptr;
        int pic1_size = 0;
        u8 *snd0_data = nullptr;
        int snd0_size = 0;
        char *title = nullptr;
    } EbootMeta;

    enum MetadataType {
        MetadataIcon0,
        MetadataIcon1,
        MetadataPic0,
        MetadataPic1,
        MetadataSnd0
    };

    static int ReadISOFile(const std::string &path, u8 **data, const std::string &file) {
        int type = 0, pos = 0, size = 0, size_pos = 0, ret = 0;
        u32 magic = 0;
        
        ISO::Read(&magic, path.c_str(), 0, sizeof(magic));
        if (magic == 0x4F534943)
            type = 1;
            
        if (R_FAILED(ret = ISO::GetInfo(&pos, &size, &size_pos, path, type, file.c_str())))
            return ret;
            
        *data = new u8[size];
        ISO::ReadFile(*data, path.c_str(), type, pos, size);
        return size;
    }

    static char *ReadISOTitle(u8 *sfo_param, const char *name, int size) {
        char *title = new char[size];
        SFO *header = reinterpret_cast<SFO *>(sfo_param);
        SFOIndex *entries = reinterpret_cast<SFOIndex *>((reinterpret_cast<u32>(header) + sizeof(SFO)));
        
        for(u32 i = 0; i < header->pair_count; i++) {
            if (std::strcmp(reinterpret_cast<char *>(reinterpret_cast<u32>(header) + header->key_offset + entries[i].key_offset), name) == 0) {
                std::memset(title, 0, size);
                std::strncpy(title, reinterpret_cast<const char *>(reinterpret_cast<u32>(header) + header->value_offset + entries[i].data_offset), size);
                title[size - 1] = '\0';
            }
        }
        
        return title;
    }

    static int ReadSFOTitle(SceUID file, u8 *buffer, int size, char *id_buf, int id_size) {
        int ret = 0;
        SFO *sfo_data = reinterpret_cast<SFO *>(buffer);
        
        // Read the SFO header
        if (R_FAILED(ret = sceIoRead(file, sfo_data, sizeof(SFO)))) {
            Log::Error("GameLauncher::ReadSFOTitle sceIoRead sfo header failed: 0x%08x\n", ret);
            return ret;
        }
        
        // Allocate memory to read the SFO block
        u8 *sfo_block = buffer + size;
        if (R_FAILED(ret = sceIoRead(file, sfo_block, size))) {
            Log::Error("GameLauncher::ReadSFOTitle sceIoRead sfo block failed: 0x%08x\n", ret);
            return ret;
        }
        
        // Get the SFO index table inside the block
        SFOIndex *index_block = reinterpret_cast<SFOIndex *>(sfo_block);
        u32 keys_offset_block = sizeof(SFO) + (sizeof(SFOIndex) * sfo_data->pair_count);
        u8 *value_block = sfo_block + sfo_data->value_offset - sizeof(SFO);

        for (u32 i = 0; i < sfo_data->pair_count; i++) {
            char *key_addr = reinterpret_cast<char *>(sfo_block) + index_block[i].key_offset + keys_offset_block - sizeof(SFO);
            if (!std::strcmp(key_addr, "TITLE")) {
                std::memcpy(id_buf, value_block, id_size);
                return id_size;
            }
            
            value_block += index_block[i].value_size_with_padding;
        }
        
        return 0;
    }

    static int GetISOMeta(const std::string &path, EbootMeta *meta) {
        int ret = 0;
        u8 *sfo_data = nullptr;

        if (R_SUCCEEDED(ret = GameLauncher::ReadISOFile(path, &sfo_data, "PSP_GAME/PARAM.SFO"))) {
            meta->title = GameLauncher::ReadISOTitle(sfo_data, "TITLE", 52);
            delete[] sfo_data;
        }

        meta->icon0_size = GameLauncher::ReadISOFile(path, &meta->icon0_data, "PSP_GAME/ICON0.PNG");
        meta->icon1_size = GameLauncher::ReadISOFile(path, &meta->icon1_data, "PSP_GAME/ICON1.PMF");
        meta->pic0_size = GameLauncher::ReadISOFile(path, &meta->pic0_data, "PSP_GAME/PIC0.PNG");
        meta->pic1_size = GameLauncher::ReadISOFile(path, &meta->pic1_data, "PSP_GAME/PIC1.PNG");
        meta->snd0_size = GameLauncher::ReadISOFile(path, &meta->snd0_data, "PSP_GAME/SND0.AT3");
        return 0;
    }
    
    static int GetEbootMeta(const std::string &path, EbootMeta *meta) {
        int ret = 0;
        char title_buf[128] = {0};
        SceUID file = 0;
        PBPHeader pbp_data = { { 0 } };
        
        if (R_FAILED(ret = file = sceIoOpen(path.c_str(), PSP_O_RDONLY, 0777))) {
            Log::Error("GameLauncher::GetEbootMeta sceIoOpen(%s) failed: 0x%08x\n", path.c_str(), ret);
            return ret;
        }
            
        sceIoRead(file, &pbp_data, sizeof(PBPHeader));
        
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
                Log::Error("GameLauncher::GetEbootMeta icon0 sceIoRead(%s) failed: 0x%08x\n", path.c_str(), ret);
                return ret;
            }
        }

        // Get icon1
        sceIoLseek(file, pbp_data.icon1_offset, PSP_SEEK_SET);
        meta->icon1_size = pbp_data.pic0_offset - pbp_data.icon1_offset;
        if (meta->icon1_size) {
            meta->icon1_data = new u8[meta->icon1_size];
            if (R_FAILED(ret = sceIoRead(file, meta->icon1_data, meta->icon1_size))) {
                Log::Error("GameLauncher::GetEbootMeta icon1 sceIoRead(%s) failed: 0x%08x\n", path.c_str(), ret);
                return ret;
            }
        }

        // Get pic0
        sceIoLseek(file, pbp_data.pic0_offset, PSP_SEEK_SET);
        meta->pic0_size = pbp_data.pic1_offset - pbp_data.pic0_offset;
        if (meta->pic0_size) {
            meta->pic0_data = new u8[meta->pic0_size];
            if (R_FAILED(ret = sceIoRead(file, meta->pic0_data, meta->pic0_size))) {
                Log::Error("GameLauncher::GetEbootMeta pic0 sceIoRead(%s) failed: 0x%08x\n", path.c_str(), ret);
                return ret;
            }
        }
        
        // Get pic1
        sceIoLseek(file, pbp_data.pic1_offset, PSP_SEEK_SET);
        meta->pic1_size = pbp_data.snd0_offset - pbp_data.pic1_offset;
        if (meta->pic1_size) {
            meta->pic1_data = new u8[meta->pic1_size];
            if (R_FAILED(ret = sceIoRead(file, meta->pic1_data, meta->pic1_size))) {
                Log::Error("GameLauncher::GetEbootMeta pic1 sceIoRead(%s) failed: 0x%08x\n", path.c_str(), ret);
                return ret;
            }
        }

        // Get snd0
        sceIoLseek(file, pbp_data.snd0_offset, PSP_SEEK_SET);
        meta->snd0_size = pbp_data.psp_offset - pbp_data.snd0_offset;
        if (meta->snd0_size) {
            meta->snd0_data = new u8[meta->snd0_size];
            if (R_FAILED(ret = sceIoRead(file, meta->snd0_data, meta->snd0_size))) {
                Log::Error("GameLauncher::GetEbootMeta snd0 sceIoRead(%s) failed: 0x%08x\n", path.c_str(), ret);
                return ret;
            }
        }
        
        sceIoClose(file);
        return 0;
    }

    static void ExportData(const std::string &path, const char *title, const std::string &ext, u8 *data, int size) {
        std::string new_path = Utils::IsInternalStorage()? "ef0:" : "ms0:";
        new_path.append(path);
        new_path.append(title);
        
        if (!(FS::DirExists(new_path)))
            FS::RecursiveMakeDir(new_path);
        
        new_path.append(ext); // "/icon0.png"
        FS::WriteFile(new_path, data, size);
    }

    int DisplayLauncher(const std::string &path) {
        int ret = 0;
        SceIoStat stat;
        
        if (R_FAILED(ret = sceIoGetstat(path.c_str(), &stat))) {
            Log::Error("DisplayLauncher sceIoGetstat(%s) failed: 0x%08x\n", path.c_str(), ret);
            return ret;
        }

        EbootMeta meta = { 0 };

        std::string ext = FS::GetFileExt(path);


        // ISO/CSO
        if ((!ext.compare(".ISO")) || (!ext.compare(".CSO"))) {
            if (R_FAILED(ret = GameLauncher::GetISOMeta(path, &meta)))
                return ret;
        }
        else {
            if (R_FAILED(ret = GameLauncher::GetEbootMeta(path, &meta)))
                return ret;
        }

        g2dTexture *icon0 = nullptr;
        if (meta.icon0_data)
            icon0 = Textures::LoadImageBufferPNG(meta.icon0_data, meta.icon0_size);
        
        char install_date[128] = {0};
        const char *months[] = { "Dec", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov" };
        snprintf(install_date, 128, "Installed %d %s %d", stat.st_ctime.day, months[stat.st_ctime.month], stat.st_ctime.year);

        int selection = 0;
        const char *metadata_types[] = { "< ICON0.PNG >", "< ICON1.PMF >", "< PIC0.PNG >", "< PIC1.PNG >", "< SND0.AT3 >" };

        while(true) {
            g2dClear(G2D_RGBA(32, 33, 36, 255));
            G2D::DrawRect(240, 10, 2, 262, G2D_RGBA(78, 80, 85, 255));
            G2D::DrawRect(242, 146, 238, 2, G2D_RGBA(78, 80, 85, 255));
            G2D::DrawImage(ic_play_btn, 312, 125);
            
            if (icon0)
                G2D::DrawImage(icon0, 50, 56);
            else {
                G2D::DrawRect(50, 56, 144, 80, G2D_RGBA(46, 46, 50, 255));
                G2D::FontSetStyle(0.75f, G2D_RGBA(232, 234, 238, 255), INTRAFONT_ALIGN_LEFT);
                G2D::DrawText(50 + ((144 - intraFontMeasureText(font, "ICON0 not found")) / 2), 96, "ICON0 not found");
            }
            
            G2D::DrawRect(260, 190, 200, 50, G2D_RGBA(46, 46, 50, 255));
            
            G2D::FontSetStyle(0.9f, G2D_RGBA(232, 234, 238, 255), INTRAFONT_ALIGN_LEFT);
            G2D::DrawText(242 + ((238 - intraFontMeasureText(font, meta.title)) / 2), 56, meta.title);
            G2D::FontSetStyle(0.75f, G2D_RGBA(232, 234, 238, 255), INTRAFONT_ALIGN_LEFT);
            G2D::DrawText(242 + ((238 - intraFontMeasureText(font, install_date)) / 2), 76, install_date);

            G2D::DrawText(242 + ((238 - intraFontMeasureText(font, "Press Square to save:")) / 2), 210, "Press Square to save:");
            G2D::DrawText(242 + ((238 - intraFontMeasureText(font, metadata_types[selection])) / 2), 230, metadata_types[selection]);
            g2dFlip(G2D_VSYNC);

            int ctrl = Utils::ReadControls();
            
            if (ctrl & PSP_CTRL_LEFT)
                selection--;
            else if (ctrl & PSP_CTRL_RIGHT)
                selection++;

            Utils::SetBounds(selection, 0, 4);
            
            if (Utils::IsButtonPressed(PSP_CTRL_ENTER))
                Utils::LaunchEboot(path.c_str());
            
            if (Utils::IsButtonPressed(PSP_CTRL_SQUARE)) {
                switch(selection) {
                    case MetadataIcon0:
                        if (meta.icon0_data)
                            GameLauncher::ExportData("/PSP/PHOTO/", meta.title, "/ICON0.PNG", meta.icon0_data, meta.icon0_size);
                        break;

                    case MetadataIcon1:
                        if (meta.icon1_data)
                            GameLauncher::ExportData("/PSP/PHOTO/", meta.title, "/ICON1.PMF", meta.icon1_data, meta.icon1_size);
                        break;

                    case MetadataPic0:
                        if (meta.pic0_data)
                            GameLauncher::ExportData("/PSP/PHOTO/", meta.title, "/PIC0.PNG", meta.pic0_data, meta.pic0_size);
                        break;

                    case MetadataPic1:
                        if (meta.pic1_data)
                            GameLauncher::ExportData("/PSP/PHOTO/", meta.title, "/PIC1.PNG", meta.pic1_data, meta.pic1_size);
                        break;

                    case MetadataSnd0:
                        if (meta.snd0_data)
                            GameLauncher::ExportData("/PSP/MUSIC/", meta.title, "/SND0.AT3", meta.snd0_data, meta.snd0_size);
                        break;
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

        if (meta.icon1_data)
            delete[] meta.icon1_data;

        if (meta.pic0_data)
            delete[] meta.pic0_data;

        if (meta.pic1_data)
            delete[] meta.pic1_data;

        if (meta.snd0_data)
            delete[] meta.snd0_data;

        return 0;
    }
}
