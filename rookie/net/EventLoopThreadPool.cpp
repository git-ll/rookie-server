

#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"

using namespace rookie;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop, int threadNum)
                    :baseloop_(baseloop),
                     threadNum_(threadNum),
                     next_(0)
{
    if(threadNum<0)
        LOG_ERROR<<"EventLoopThreadPool::EventLoopThreadPool threadnum less than 0.";
    LOG_DEBUG<<"EventLoopThreadPool create. size : "<<threadNum_;
}

EventLoopThreadPool::~EventLoopThreadPool()
{

}

void EventLoopThreadPool::start()
{
    for(int i=0;i<threadNum_;i++)
    {
        auto t = new EventLoopThread();
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());  //在startLoop中已经保证了子线程的eventLoop会先于主线程运行起来，因此最终子线程的loops肯定都先于主线程loop运行
    }
    LOG_INFO<<threadNum_<<" io threads running now.";
}

EventLoop* EventLoopThreadPool::nextLoop()
{
    EventLoop* loop_ = baseloop_;
    if(!loops_.empty())
    {
        loop_ = loops_[next_++];
        if(next_ == threadNum_)
        {
            next_ = 0;
        }
    }
    return loop_;
}
