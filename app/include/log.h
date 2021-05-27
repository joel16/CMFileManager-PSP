#ifndef _CMFILEMANAGER_LOG_H_
#define _CMFILEMANAGER_LOG_H_

namespace Log {
    int OpenHande(void);
    int CloseHandle(void);
    int Error(const char *format, ...);
}

#endif
