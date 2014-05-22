#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header/timeOperate.h"
const char* now()
{
    static char str[30];
    struct tm* pTm;

    time_t seconds = time(NULL);
    pTm = localtime(&seconds);

    sprintf(str,"%02d-%02d-%02d %02d:%02d:%02d",pTm->tm_year+1900,pTm->tm_mon+1,pTm->tm_mday,pTm->tm_hour,pTm->tm_min,pTm->tm_sec);

    return str;
}

const char* nowNoSignal()
{
    static char str[30];
    struct tm* pTm;

    time_t seconds = time(NULL);
    pTm = localtime(&seconds);

    sprintf(str,"%02d%02d%02d%02d%02d%02d",pTm->tm_year+1900,pTm->tm_mon+1,pTm->tm_mday,pTm->tm_hour,pTm->tm_min,pTm->tm_sec);

    return str;
}
