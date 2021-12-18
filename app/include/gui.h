#ifndef _CMFILEMANAGER_GUI_HELPER_H_
#define _CMFILEMANAGER_GUI_HELPER_H_

#include <glib2d.h>
#include <pspiofilemgr.h>
#include <string>
#include <vector>

enum MENU_STATES {
    MENU_STATE_MENUBAR,
    MENU_STATE_FILEBROWSER,
    MENU_STATE_OPTIONS,
    MENU_STATE_DELETE,
    MENU_STATE_PROPERTIES,
    MENU_STATE_SETTINGS,
    MENU_STATE_IMAGEVIEWER,
    MENU_STATE_ARCHIVEEXTRACT,
    MENU_STATE_TEXTREADER
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
    g2dTexture *texture = nullptr;
} MenuItem;

extern bool g_running;

namespace GUI {
    void ResetCheckbox(MenuItem &itemitem);
    void GetStorageSize(MenuItem &itemitem);
    void DisplayStatusBar(void);
    void ProgressBar(const std::string &title, std::string message, u64 offset, u64 size);
    int RenderLoop(void);

    void HandleMenubarAnim(float &delta);
    void DisplayMenubar(void);
    void ControlMenubar(MenuItem &itemitem, int &ctrl);

    void DisplayFileBrowser(MenuItem &itemitem);
    void ControlFileBrowser(MenuItem &itemitem, int &ctrl);

    void DisplayFileOptions(MenuItem &itemitem);
    void ControlFileOptions(MenuItem &itemitem, int &ctrl);

    void DisplayFileProperties(MenuItem &itemitem);
    void ControlFileProperties(MenuItem &itemitem);

    void DisplayDeleteOptions(void);
    void ControlDeleteOptions(MenuItem &itemitem, int &ctrl);

    void DisplaySettings(MenuItem &itemitem);
    void ControlSettings(MenuItem &itemitem, int &ctrl);

    void DisplayImageViewer(MenuItem &itemitem);
    void ControlImageViewer(MenuItem &itemitem, float &delta);
}

#endif
