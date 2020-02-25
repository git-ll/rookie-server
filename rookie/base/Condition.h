

#ifndef ROOKIE_CONDITION_H
#define ROOKIE_CONDITION_H

#include "noncopyable.h"
#include "Mutex.h"
#include <errno.h>
#include "LogOutput.h"

namespace rookie
{
    class Condition: noncopyable
    {
    public:
        explicit Condition(MutexLock &_mutex):
                mutex(_mutex)
        {
            pthread_cond_init(&cond, nullptr);
        }
        ~Condition()
        {
            pthread_cond_destroy(&cond);
        }
        void wait()
        {
            if(pthread_cond_wait(&cond, mutex.getMutex()))
                LOG_ERROR<<"wait()";
        }
        void notify()
        {
            if(pthread_cond_signal(&cond))
                LOG_ERROR<<"notify()";
        }
        void notifyAll()
        {
            if(pthread_cond_broadcast(&cond))
                LOG_ERROR<<"notifyAll()";
        }
        bool waitForSeconds(int seconds)
        {
            struct timespec abstime = {};
            clock_gettime(CLOCK_REALTIME, &abstime); //和gettimeofday一样
            abstime.tv_sec += static_cast<time_t>(seconds);//计算超时的时刻
            return ETIMEDOUT == pthread_cond_timedwait(&cond, mutex.getMutex(), &abstime);//如果返回ETIMEDOUT，说明条件变量超时仍未被唤醒，唤醒后重新尝试获取锁
            //return pthread_cond_timedwait(&cond, mutex.getMutex(), &abstime);
        }
    private:
        MutexLock &mutex;
        pthread_cond_t cond;
    };
}




#endif //ROOKIE_CONDITION_H
