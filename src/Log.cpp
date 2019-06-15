#include <stdio.h>

#include "ILog.h"

FILE *F = NULL;

int ILog::init(const char* fileName)
{
    if (!fileName) return ERR_WRONG_ARG;
    FILE *tmp = fopen(fileName, "w");
    if (!tmp) return ERR_OPEN_ILogImpl;
    if (F) fclose(F);
    F = tmp;
    return ERR_OK;
}

int ILog::report(const char *msg)
{
    if (!msg) return ERR_WRONG_ARG;
    if (!F || fprintf(F, msg) < 0) return ERR_WRITE_TO_ILogImpl;
    return ERR_OK;
}

void ILog::destroy()
{
    if (F) fclose(F);
    F = NULL;
}
