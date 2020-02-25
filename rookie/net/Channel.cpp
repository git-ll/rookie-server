

#include "Channel.h"
#include "LogOutput.h"
#include "EventLoop.h"
#include "Util.h"
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <assert.h>

using namespace rookie;

Channel::Channel(int fd, rookie::EventLoop *evloop)
        :fd_(fd),
         evloop_(evloop),
         events_(),
         revents_(EV_NONE),
         ReadCallBack_(nullptr),
         WriteCallBack_(nullptr),
         ErrorCallBack_(nullptr),
         CloseCallBack_(nullptr),
         state_(EVSTATE_NONE)
{
    if(!evloop_)
        LOG_FATAL<<"EventLoop is nullptr!";
    memset(&timeout_,0, sizeof(timeval));
}

void Channel::handleChannel()
{
    LOG_DEBUG<<"Active events with fd "<<fd_<<':'<<eventsToString(revents_)<<revents_;
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN) )   //对端关闭不会触发这条，需要在readcallback中处理
    {
        LOG_WARN<<"Channel::handleChannel EPOLLHUP";
        if (CloseCallBack_) CloseCallBack_();
    }
    if (revents_ & EPOLLERR)
    {
        if(errno == EAGAIN)
        {
            return;
        }
        LOG_FATAL<<"Channel::handleChannel EPOLLERR with fd "<<fd_<<" add ERROR is "<<errno;
        if (ErrorCallBack_) ErrorCallBack_();
        return;
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        if(ReadCallBack_)ReadCallBack_();
    }
    if (revents_ & EPOLLOUT)
    {
        if(WriteCallBack_)WriteCallBack_();
    }
}

void Channel::enableChannel(uint32_t flags,int timeout)
{
    assert(flags&EV_ALL);
    assert(timeout==-1 || timeout>=0);

    events_ = flags;

    if(timeout!=-1)  //如果设置了超时时间，就计算其超时的时刻
    {
        gettimeofday(&timeout_, nullptr);
        timeout_=timeadd(timeout_,timeout);
        events_ |= EV_TIMEOUT;   //EV_TIMEOUT表示channel是一个超时事件
    }
    else if(events_ == EV_TIMEOUT)  //纯超时Channel必须指定超时时间
    {
        LOG_WARN<<"Channel::enableChannel EV_TIMEOUT channel should specify param 'timeout' ";
        return;
    }
    evloop_->runInLoop(std::bind(&EventLoop::updateChannle,evloop_,this,timeout!=-1));//将updateChannel放到io线程中执行，确保Timer和ChannelMap的线程安全
}

void Channel::disableChannel()
{
    evloop_->runInLoop(std::bind(&EventLoop::removeChannle,evloop_,this));  //将removeChannel放到io线程中执行，确保Timer和ChannelMap的线程安全
    //evloop_->removeChannle(this);
}

const char* Channel::eventsToString(int events)
{
    switch(events)
    {
        case EV_NONE:return "EV_NONE";
        case EV_READ:return "EV_READ";
        case EV_WRITE:return "EV_WRITE";
        case EV_TIMEOUT:return "EV_TIMEOUT";
        case EV_READ|EV_WRITE:return "EV_READ | EV_WRITE";
        case EV_READ|EV_TIMEOUT:return "EV_READ | EV_TIMEOUT";
        case EV_WRITE|EV_TIMEOUT:return "EV_WRITE | EV_TIMEOUT";
        case EV_READ|EV_WRITE|EV_TIMEOUT:return "EV_READ | EV_WRITE | EV_TIMEOUT";
        default:
            return "NODEF";
    }
}