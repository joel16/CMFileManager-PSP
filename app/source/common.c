#include "common.h"

jmp_buf exitJmp;

int MENU_STATE;
int BROWSE_STATE;

intraFont *font, *jpn_font, *chn_font, *kor_font, *sym_font;

char cwd[512];
char root_path[10];
char initial_cwd[128];

bool is_ms_inserted;
bool is_psp_go;

// display driver function prototypes
int pspDisplayEnable(void);
int pspDisplayDisable(void);
