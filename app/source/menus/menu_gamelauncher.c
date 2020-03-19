#include <malloc.h>
#include <stdio.h>

#include "common.h"
#include "fs.h"
#include "glib2d_helper.h"
#include "screenshot.h"
#include "textures.h"
#include "utils.h"

typedef struct {
    char id[4];
    unsigned int version;
    unsigned int sfo_offset;
    unsigned int icon0_offset;
    unsigned int icon1_offset;
    unsigned int pic0_offset;
    unsigned int pic1_offset;
    unsigned int snd0_offset;
    unsigned int psp_offset;
    unsigned int psar_offset;
} pbp;

typedef struct {
    unsigned short key_offset;
    unsigned char alignment;
    unsigned char data_type;
    unsigned int value_size;
    unsigned int value_size_with_padding;
    unsigned int data_offset;
} sfo_index;

typedef struct {
    char id[4];
    unsigned int version;
    unsigned int key_offset;
    unsigned int value_offset;
    unsigned int pair_count;
} sfo;

typedef struct {
    unsigned char *icon0_data;
    int icon0_size;
    unsigned char *pic1_data;
    int pic1_size;
    char *title;
} eboot_meta;

// From https://github.com/codestation/prxshot/blob/master/sfo.c#L48
static int Game_ReadSFOTitle(SceUID file, char *buffer, int size, char *id_buf, int id_size) {
    sfo *sfo_data = (sfo *)buffer;
    // read the sfo header
    sceIoRead(file, sfo_data, sizeof(sfo));
    // allocate memory to read the sfo block
    void *sfo_block = buffer + size;
    sceIoRead(file, sfo_block, size);
    // get the sfo index table inside the block
    sfo_index *index_block = sfo_block;
    unsigned int keys_offset_block = sizeof(sfo) + (sizeof(sfo_index) * sfo_data->pair_count);
    void *value_block = sfo_block + sfo_data->value_offset - sizeof(sfo);

    int i = 0;
    for (i = 0; i < sfo_data->pair_count; i++) {
        char *key_addr = sfo_block + index_block[i].key_offset + keys_offset_block - sizeof(sfo);
        if (!strcmp(key_addr, "TITLE")) {
            memcpy(id_buf, value_block, id_size);
            return id_size;
        }
        
        value_block += index_block[i].value_size_with_padding;
    }
    
    return 0;
}

static bool Game_GetPBPMeta(const char *path, eboot_meta *meta) {
    char title_buf[128];
    SceUID file = 0;
    pbp pbp_data = {{0}};
    
    if (R_FAILED(file = sceIoOpen(path, PSP_O_RDONLY, 0777)))
        return false;
        
    sceIoRead(file, &pbp_data, sizeof(pbp));
    
    // Get title
    int title_size = pbp_data.icon0_offset - pbp_data.sfo_offset;
    void *buffer = NULL;
    buffer = malloc(4096);
    meta->title = (char *)calloc(128, sizeof(char));
    Game_ReadSFOTitle(file, buffer, title_size, title_buf, sizeof(title_buf));
    snprintf(meta->title, 128, title_buf);
    free(buffer);
    
    // Get icon0
    sceIoLseek(file, pbp_data.icon0_offset, PSP_SEEK_SET);
    meta->icon0_size = pbp_data.icon1_offset - pbp_data.icon0_offset;
    if (meta->icon0_size) {
        meta->icon0_data = malloc(meta->icon0_size);
        sceIoRead(file, meta->icon0_data, meta->icon0_size);
    }
    
    // Get pic1
    sceIoLseek(file, pbp_data.pic1_offset, PSP_SEEK_SET);
    meta->pic1_size = pbp_data.snd0_offset - pbp_data.pic1_offset;
    if (meta->pic1_size) {
        meta->pic1_data = malloc(meta->pic1_size);
        sceIoRead(file, meta->pic1_data, meta->pic1_size);
    }
    
    sceIoClose(file);
    return false;
}

void Game_DisplayLauncher(const char *path) {
    SceIoStat stat;
    if (R_FAILED(sceIoGetstat(path, &stat)))
        return;
        
    const char *months[] = {
        "Dec",
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sept",
        "Oct",
        "Nov"
    };
    
    g2dTexture *icon0 = NULL;
    eboot_meta meta = { 0 };
    Game_GetPBPMeta(path, &meta);
    icon0 = g2dTexLoadMemory(meta.icon0_data, meta.icon0_size, G2D_SWIZZLE);
    
    char install_date[128];
    snprintf(install_date, 128, "Installed %d %s %d", stat.st_ctime.day, months[stat.st_ctime.month], stat.st_ctime.year);

    while (1) {
        g2dClear(G2D_RGBA(32, 33, 36, 255));
        G2D_DrawRect(240, 10, 2, 262, G2D_RGBA(78, 80, 85, 255));
        G2D_DrawRect(242, 146, 238, 2, G2D_RGBA(78, 80, 85, 255));
        G2D_DrawImage(ic_play_btn, 312, 125);
        
        if (icon0)
            G2D_DrawImage(icon0, 50, 56);
        else {
            G2D_DrawRect(50, 56, 144, 80, G2D_RGBA(46, 46, 50, 255));
            intraFontSetStyle(font, 0.65f, G2D_RGBA(232, 234, 238, 255), G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
            intraFontPrint(font, 50 + ((144 - intraFontMeasureText(font, "ICON0 not found")) / 2), 96, "ICON0 not found");
        }

        G2D_DrawRect(260, 190, 200, 50, G2D_RGBA(46, 46, 50, 255));
        
        intraFontSetStyle(font, 0.7f, G2D_RGBA(232, 234, 238, 255), G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
        intraFontPrint(font, 242 + ((238 - intraFontMeasureText(font, meta.title)) / 2), 56, meta.title);
        intraFontSetStyle(font, 0.65f, G2D_RGBA(232, 234, 238, 255), G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
        intraFontPrint(font, 242 + ((238 - intraFontMeasureText(font, install_date)) / 2), 76, install_date);

        intraFontPrint(font, 242 + ((238 - intraFontMeasureText(font, "Press Square to save ICON0")) / 2), 210, "Press Square to save ICON0");
        intraFontPrint(font, 242 + ((238 - intraFontMeasureText(font, "Press Triangle to save PIC1")) / 2), 230, "Press Triangle to save PIC1");
        g2dFlip(G2D_VSYNC);
        
        Utils_ReadControls();
        
        if (Utils_IsButtonPressed(PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER))
            Screenshot_Capture();
        
        if (Utils_IsButtonPressed(PSP_CTRL_ENTER))
            Utils_LaunchEboot(path);

        if (Utils_IsButtonPressed(PSP_CTRL_SQUARE)) {
            if (meta.icon0_size) {
                char icon0_path[160];
                snprintf(icon0_path, 160, "%s%s", Utils_IsEF0()? "ef0:/PSP/PHOTO/CMFileManager/" : "ms0:/PSP/PHOTO/CMFileManager/", meta.title);
                
                if (!(FS_DirExists(icon0_path)))
                    FS_RecursiveMakeDir(icon0_path);

                strcat(icon0_path, "/icon0.png");
                FS_WriteFile(icon0_path, meta.icon0_data, meta.icon0_size);
            }
        }

        if (Utils_IsButtonPressed(PSP_CTRL_TRIANGLE)) {
            if (meta.pic1_size) {
                char pic1_path[160];
                snprintf(pic1_path, 160, "%s%s", Utils_IsEF0()? "ef0:/PSP/PHOTO/CMFileManager/" : "ms0:/PSP/PHOTO/CMFileManager/", meta.title);
                
                if (!(FS_DirExists(pic1_path)))
                    FS_RecursiveMakeDir(pic1_path);

                strcat(pic1_path, "/pic1.png");
                FS_WriteFile(pic1_path, meta.pic1_data, meta.pic1_size);
            }
        }
            
        if (Utils_IsButtonPressed(PSP_CTRL_CANCEL))
            break;
    }

    if (icon0)
        g2dTexFree(&icon0);
    
    if (meta.title)
        free(meta.title);

    if (meta.icon0_data)
        free(meta.icon0_data);
}
