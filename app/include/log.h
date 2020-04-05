#ifndef CMFILEMANAGER_LOG_H
#define CMFILEMANAGER_LOG_H

int Log_OpenFileHande(void);
int Log_CloseFileHandle(void);
int Log_Print(const char *format, ...);

#endif
