

#ifndef ROOKIE_EVENTTHREADPOOL_H
#define ROOKIE_EVENTTHREADPOOL_H

#include "../base/noncopyable.h"
#include <memory>
#include <vector>

namespace rookie
{
    class EventLoop;
    class EventLoopThread;
    class EventLoopThreadPool:noncopyable
    {
    public:
        EventLoopThreadPool(EventLoop* baseloop,int threadNum);
        ~EventLoopThreadPool();

        void start();
        EventLoop* nextLoop();

    private:
        EventLoop* baseloop_;   //主loop，负责监听连接
        std::vector<std::unique_ptr<EventLoopThread>>threads_;   //EventLoopThreadPool负责EventLoopThread的生命周期
        std::vector<EventLoop*>loops_;    //不负责EventLoop的释放，EventLoop的生命周期由EventLoopThread控制
        int threadNum_;
        int next_;
    };
}

#endif //ROOKIE_EVENTTHREADPOOL_H
