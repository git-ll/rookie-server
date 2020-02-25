

#include "CountDownLatch.h"

using namespace rookie;

CountDownLatch::CountDownLatch(int count)   //一个计数器类，当计数值大于0时休眠，等于0时唤醒
  : mutex_(),
    condition_(mutex_),
    count_(count)
{
}

void CountDownLatch::wait()
{
  MutexLockGuard lock(mutex_);
  while (count_ > 0)  //如果count大于0就休眠
  {
    condition_.wait();
  }
}

void CountDownLatch::countDown()
{
  MutexLockGuard lock(mutex_);
  --count_;
  if (count_ == 0)//如果count为0了就唤醒
  {
    condition_.notifyAll();
  }
}

int CountDownLatch::getCount() const
{
  MutexLockGuard lock(mutex_);
  return count_;
}

