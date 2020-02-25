

#ifndef ROOKIE_EVENTLOOPTHREAD_H
#define ROOKIE_EVENTLOOPTHREAD_H

#include "../base/noncopyable.h"
#include "../base/Mutex.h"
#include "../base/Condition.h"
#include <memory>

namespace rookie
{
    class EventLoop;
    class Thread;
    class EventLoopThread:noncopyable    //对应一个线程，线程函数中运行Eventloop，负责Eventloop的创建和销毁
    {
    public:
        explicit EventLoopThread();
        ~EventLoopThread();
        EventLoop* startLoop();

    private:
        void threadFun();
        bool looping_;
        //std::unique_ptr<EventLoop>loop_;
        EventLoop* loop_;    //实际上指向的是子线程中的栈对象，不控制其生命周期
        std::unique_ptr<Thread>thread_;
        MutexLock mutex_;
        Condition cv_;
    };

    /*class EventLoopThread :noncopyable
    {
    public:
        EventLoopThread();
        ~EventLoopThread();
        EventLoop* startLoop();

    private:
        void threadFunc();
        EventLoop *loop_;
        Thread thread_;
        MutexLock mutex_;
        Condition cond_;
    };*/
}




#endif //ROOKIE_EVENTLOOPTHREAD_H
