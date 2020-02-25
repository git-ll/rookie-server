

#ifndef ROOKIE_CURRENTTHREADINFO_H
#define ROOKIE_CURRENTTHREADINFO_H

#include "noncopyable.h"
#include <string>
#include <sys/syscall.h>
#include <unistd.h>

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

namespace rookie
{
    class ThreadLocalInfo : noncopyable    //用来保存当前线程的信息，无需创建线程对象，也不用频繁调用getpid()这一类的函数
    {
    public:
        static int tid()
        {
            if(LIKELY(TL_cachedTid == 0))
            {
                TL_cachedTid = static_cast<int>(::syscall(SYS_gettid));
                TL_tidStringLength = snprintf(TL_tidString, sizeof(TL_tidString), "%5d ", TL_cachedTid);
            }
            return TL_cachedTid;
        }

        static const char* name(){ return TL_threadName; }
        static int tidStringLength(){ return TL_tidStringLength;}
        bool isMainThread(){ return tid() == getpid();}

        static __thread const char* TL_threadName;    //线程刚开始时设置为自定义的线程名，而后"finish"表示线程函数执行结束，"crash"表示出现异常

    private:
        static __thread int TL_cachedTid;
        static __thread char TL_tidString[32];
        static __thread int TL_tidStringLength;

    };

}

#endif //ROOKIE_CURRENTTHREADINFO_H
