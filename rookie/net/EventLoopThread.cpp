

#include "EventLoopThread.h"
#include "EventLoop.h"
#include "Thread.h"
#include "LogOutput.h"

using namespace rookie;


EventLoopThread::EventLoopThread()
                ://loop_(new EventLoop()), //从堆创建的EventLoop依然是属于主线程的，那么每个EventLoop的threadid_都是相同的，应该在线程函数栈中创建栈变量EventLoop
                 loop_(nullptr),
                 thread_(new Thread(std::bind(&EventLoopThread::threadFun,this))),
                 looping_(false),
                 mutex_(),
                 cv_(mutex_)
{

}

EventLoopThread::~EventLoopThread()
{
        if(loop_)
        {
                LOG_INFO << "Wait for EventLoopThread " << thread_->name()<<" exit";
                loop_->exit();
                thread_->join();
                LOG_INFO << "EventLoopThread "<< thread_->name()<<" exit";
        }
}

EventLoop* EventLoopThread::startLoop()
{
        thread_->start();
        {
                MutexLockGuard m(mutex_);      //必须使用锁来保证wait和notify的同步，否则可能等待线程执行到判断looping和wait之间，此时切换到唤醒线程执行了notify，此时由于还未执行wait，因此唤醒丢失，等到真正执行wait时就无法唤醒了
                while(!loop_)
                {
                        cv_.wait();
                }
        }
        while(!loop_->isLooping());   //确保子线程的loop已经开始执行
        return loop_;
}

void EventLoopThread::threadFun()    //运行于子线程
{
        EventLoop loop;
        {
                MutexLockGuard m(mutex_);
                loop_ = &loop;
                cv_.notify(); //如何保证notify之后wait的下一次判断不会抢在loop()之前？不然的话，由于还未进入loop，wait苏醒后检查条件还是不符合又会陷入睡眠
        }
        loop_->loop();
        loop_ = nullptr;
}


