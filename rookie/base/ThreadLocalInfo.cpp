

#include "ThreadLocalInfo.h"

namespace rookie
{
    __thread int ThreadLocalInfo::TL_cachedTid = 0;
    __thread char ThreadLocalInfo::TL_tidString[32];
    __thread int ThreadLocalInfo::TL_tidStringLength = 0;
    __thread const char* ThreadLocalInfo::TL_threadName = "undef";

}