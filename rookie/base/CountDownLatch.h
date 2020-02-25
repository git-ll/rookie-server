

#ifndef ROOKIE_COUNTDOWNLATCH_H
#define ROOKIE_COUNTDOWNLATCH_H

#include "Condition.h"
#include "Mutex.h"
#include "noncopyable.h"

namespace rookie
{
    class CountDownLatch : noncopyable
    {
    public:

        explicit CountDownLatch(int count);

        void wait();

        void countDown();

        int getCount() const;

    private:
        mutable MutexLock mutex_;
        Condition condition_ ;
        int count_ ;
    };

}
#endif
