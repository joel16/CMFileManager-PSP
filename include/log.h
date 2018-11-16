#pragma once

#include <pspiofilemgr.h>

#define DEBUG

int log_print(const char* format, ...);

#ifdef DEBUG
    #define DEBUG_LOG(...) log_print(__VA_ARGS__)
#else
    #define DEBUG_LOG(...)
#endif