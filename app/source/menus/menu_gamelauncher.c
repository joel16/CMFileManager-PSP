#include <malloc.h>
#include <pspiofilemgr.h>
#include <stdio.h>

#include "common.h"
#include "glib2d_helper.h"
#include "screenshot.h"
#include "textures.h"
#include "utils.h"

struct pbp {
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
}__attribute__((packed));

char *Game_GetDirname(const char *path) {
    char *token = NULL, *directory = NULL;
    size_t length = 0;
    
    token = strrchr(path, '/');
    if (token == NULL)
        return "NULL";
        
    length = strlen(token);
    directory = malloc(length);
    memcpy(directory, token+1, length);
    return directory;
}

static bool Game_GetIcon0(const char *path, g2dTexture **image, struct pbp *pbp_data) {
	SceUID fd = 0;

	if (R_FAILED(fd = sceIoOpen(path, PSP_O_RDONLY, 0777)))
		return false;
		
	sceIoRead(fd, pbp_data, sizeof(struct pbp));
	sceIoLseek(fd, pbp_data->icon0_offset, PSP_SEEK_SET);
	int icon0_size = pbp_data->icon1_offset - pbp_data->icon0_offset;
	
	unsigned char icon[icon0_size];
	if (icon0_size) {
		sceIoRead(fd, icon, icon0_size);
		*image = g2dTexLoadMemory(icon, icon0_size, G2D_SWIZZLE);
	}
	else {
		sceIoClose(fd);
		return false;
	}
	
	sceIoClose(fd);
	return true;
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
    struct pbp pbp_data = { 0 };
    bool ret = Game_GetIcon0(path, &icon0, &pbp_data);
    
    char install_date[128];
    snprintf(install_date, 128, "Installed %d %s %d", stat.st_ctime.day, months[stat.st_ctime.month], stat.st_ctime.year);

    char *dirname = NULL;
    dirname = Game_GetDirname(path);
    
    while (1) {
        g2dClear(G2D_RGBA(32, 33, 36, 255));
        G2D_DrawRect(240, 10, 2, 262, G2D_RGBA(78, 80, 85, 255));
        G2D_DrawRect(242, 146, 238, 2, G2D_RGBA(78, 80, 85, 255));
        
        if (ret)
            G2D_DrawImage(ic_play_btn, 312, 125);
        else
            G2D_DrawRect(312, 125, 144, 80, WHITE);
        
        G2D_DrawImage(icon0, 50, 56);
        
        intraFontSetStyle(font, 0.7f, G2D_RGBA(232, 234, 238, 255), G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
        intraFontPrint(font, 242 + ((238 - intraFontMeasureText(font, dirname)) / 2), 56, dirname);
        intraFontSetStyle(font, 0.65f, G2D_RGBA(232, 234, 238, 255), G2D_RGBA(0, 0, 0, 0), 0.f, INTRAFONT_ALIGN_LEFT);
        intraFontPrint(font, 242 + ((238 - intraFontMeasureText(font, install_date)) / 2), 76, install_date);
        g2dFlip(G2D_VSYNC);
        
        Utils_ReadControls();
        
        if (((Utils_IsButtonHeld(PSP_CTRL_LTRIGGER)) && (Utils_IsButtonPressed(PSP_CTRL_RTRIGGER))) || ((Utils_IsButtonHeld(PSP_CTRL_RTRIGGER)) && (Utils_IsButtonPressed(PSP_CTRL_LTRIGGER))))
            Screenshot_Capture();
        
        if (Utils_IsButtonPressed(PSP_CTRL_ENTER))
            Utils_LaunchEboot(path);
            
        if (Utils_IsButtonPressed(PSP_CTRL_CANCEL))
            break;
    }

    if (icon0)
        g2dTexFree(&icon0);

    free(dirname);
}
