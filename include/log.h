#pragma once

#include <pspiofilemgr.h>

#define DEBUG

int log_print(const char* format, ...);

#ifdef DEBUG
    #define DEBUG_PRINT(...) log_print(__VA_ARGS__)
#else
    #define DEBUG_PRINT(...)
#endif