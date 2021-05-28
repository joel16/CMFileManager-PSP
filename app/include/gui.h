#ifndef _CMFILEMANAGER_GUI_HELPER_H_
#define _CMFILEMANAGER_GUI_HELPER_H_

#include <pspiofilemgr.h>
#include <vector>

enum MENU_STATES {
    MENU_STATE_FILEBROWSER,
    MENU_STATE_OPTIONS,
    MENU_STATE_DELETE,
    MENU_STATE_PROPERTIES,
    MENU_STATE_SETTINGS,
    MENU_STATE_IMAGEVIEWER,
    MENU_STATE_ARCHIVEEXTRACT,
    MENU_STATE_TEXTREADER,
    MENU_STATE_UPDATE
};

typedef struct {
    MENU_STATES state = MENU_STATE_FILEBROWSER;
    int selected = 0;
    std::vector<SceIoDirent> entries;
    std::vector<bool> checked;
    std::vector<bool> checked_copy;
    std::string checked_cwd;
    int checked_count = 0;
    u64 used_storage = 0;
    u64 total_storage = 0;
    g2dTexture *texture;
} MenuItem;

namespace GUI {
    void ResetCheckbox(MenuItem *item);
    void GetStorageSize(MenuItem *item);
    void DisplayStatusBar(void);
    int RenderLoop(void);

    void DisplayFileBrowser(MenuItem *item);
    void ControlFileBrowser(MenuItem *item, int *ctrl);

    void DisplayFileOptions(MenuItem *item);
    void ControlFileOptions(MenuItem *item, int *ctrl);

    void DisplayFileProperties(MenuItem *item);
    void ControlFileProperties(MenuItem *item);

    void DisplayDeleteOptions(MenuItem *item);
    void ControlDeleteOptions(MenuItem *item, int *ctrl);

    void DisplaySettings(MenuItem *item);
    void ControlSettings(MenuItem *item, int *ctrl);

    void DisplayImageViewer(MenuItem *item);
    void ControlImageViewer(MenuItem *item, float *delta_time);
}

#endif
