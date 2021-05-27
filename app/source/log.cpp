#include <pspiofilemgr.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "config.h"
#include "utils.h"

namespace Log {
    SceUID log_handle = 0;

    int OpenHande(void) {
        if (R_FAILED(log_handle = sceIoOpen("debug.log", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777)))
            return log_handle;
            
        return 0;
    }

    int CloseHandle(void) {
        int ret = 0;
        
        if (R_FAILED(ret = sceIoClose(log_handle)))
            return ret;
            
        return 0;
    }

    int Error(const char *format, ...) {
        if (!cfg.dev_options)
            return -1;
            
        va_list list;
        char string[512] = {0};
        
        va_start(list, format);
        std::vsprintf(string, format, list);
        va_end(list);
        
        std::printf("%s", string);
        
        int ret = 0;
        if (R_FAILED(ret = sceIoWrite(log_handle, string, std::strlen(string))))
            return ret;
            
        return 0;
    }
}
