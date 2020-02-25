

#include "EventLoop.h"
#include <assert.h>
#include "Channel.h"
#include "Epoll.h"
#include "LogOutput.h"
#include "Timer.h"
#include "../base/ThreadLocalInfo.h"
#include <sys/eventfd.h>
#include <sys/unistd.h>
#include <Mutex.h>
#include <sys/timerfd.h>

using namespace rookie;

EventLoop::EventLoop()
          :looping_(false),
           stop_(false),
           iswaiting_(false),
           poller_(this),
           timer_(new Timer(this)),    //一个Timer实例，其中的loop指针指向该EventLoop
           wakefd_(eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC)),
           wakeChannel_(new Channel(wakefd_,this)),  //用于唤醒EventLoop所用的channel
           hbfd_(timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK)),
           hbcycle_(3*60),  //心跳间隔为3分钟
           hblimit_(5),
           hbCb(nullptr),
           hbChannel_(new Channel(hbfd_,this)),
           threadid_(ThreadLocalInfo::tid()),
           mutex_()
{
    wakeChannel_->setReadCallBack(std::bind(&EventLoop::wakeReadCallBack,this));
    wakeChannel_->enableChannel(EV_READ);
    hbChannel_->setReadCallBack(std::bind(&EventLoop::hbReadCallBack,this));
    hbChannel_->enableChannel(EV_READ);
}

EventLoop::~EventLoop()
{
    ::close(wakefd_);
    ::close(hbfd_);
}

void EventLoop::updateChannle(Channel* channel,bool timeout)
{
    assert(channel);
    if(timeout)  //由于调用该函数的线程和执行loop的io线程可能不是同一个线程，因此对于timer_就会存在线程安全问题，应该将添加定时器的操作放到io线程中进行
    {
        timer_->addTimer(channel);   //添加到定时器中
        if(channel->events_ == EV_TIMEOUT)  //如果只是个纯超时事件，就不用添加到epoll中了
            return;
    }
    //runInLoop(std::bind(&Epoll::epoll_update,&poller_,channel));//即使是超时channel也应该添加到epoll中，因为在等待超时的过程中channel也有可能被激活
    poller_.epoll_update(channel);
}

void EventLoop::removeChannle(Channel* channel)
{
    assert(channel);
    channel->events_ = EV_NONE;
    if(channel->state_ & EVSTATE_TIMEOUT)
    {
        timer_->removeTimer(channel);
        if(channel->events_ == EV_TIMEOUT)  //如果只是个纯超时事件，就不用从epoll中删除了
            return;
    }
    runInLoop(std::bind(&Epoll::epoll_remove,&poller_,channel));//放到io线程中执行，确保poller中ChannelMap的线程安全
    //poller_->epoll_remove(channel);
}

void EventLoop::loop()
{
    LOG_DEBUG<<"EventLoop start.";
    looping_ = true;
    stop_ = false;
    itimerspec val = {};
    val.it_value.tv_sec = 1;     //1s后开始定时
    val.it_interval.tv_sec = hbcycle_;    //每隔hbcycle_时间就触发一次
    timerfd_settime(hbfd_,0,&val, nullptr);
    //如果在这两句之间EventLoopThread对象析构，调用exit函数让stop_为true怎么办？
    while(!stop_)
    {
        ActiveChannels_.clear();
        int timeout = timer_->getFirstTimeOut();
        LOG_DEBUG<<"EventLoop dispatch with timeout "<<timeout;
        poller_.epoll_dispatch(timeout);
        handleTimeOuts();   //将定时器中超时的事件添加到激活队列中
        processChannels();  //处理所有激活的事件
        doPendingFunctors();
    }
    looping_ = false;
    LOG_INFO<<"EventLoop exit.";
}

bool EventLoop::isLooping()
{
    return looping_;
}

void EventLoop::exit()
{
    wakeupLoop();
    stop_ = true;
}

void EventLoop::wakeupLoop()
{
    uint64_t wakeBytes = 1;
    auto n = write(wakefd_,&wakeBytes,sizeof(wakeBytes));
    if(n!=sizeof(wakeBytes))
    {
        LOG_ERROR<<"wakefd write "<<n<<" bytes instead of "<< sizeof(wakeBytes);
    }
}

void EventLoop::handleTimeOuts()   //由epoll_dispatch调用
{
    timer_->handleTimeOuts();
}

void EventLoop::activeChannel(Channel* channel)
{
    ActiveChannels_.push_back(channel);//考虑优先级？？？
    if(channel->events_ & EV_TIMEOUT) //如果是超时事件，那么不管是否因超时而激活都将其从epoll中删除
    {
        removeChannle(channel);
        LOG_DEBUG<<"channel removed with fd "<<channel->fd();
    }
}

void EventLoop::processChannels()  //处理channel
{
    for(auto& active : ActiveChannels_)
    {
        active->handleChannel();
    }
    LOG_DEBUG<<ActiveChannels_.size()<<" channels processed.";
}

void EventLoop::runInLoop(Fun func)//这个函数可能会在其它线程中被调用
{
    if(ThreadLocalInfo::tid() == threadid_)  //如果是io线程自己调用的runInLoop，那么直接执行Func即可（线程池大小为0的情况）
    {
        func();
    }
    else    //调用的线程不是相应的io线程，就需要唤醒io线程
    {
        {
            MutexLockGuard m(mutex_);
            pendingFunctors.push_back(std::move(func));
        }
        wakeupLoop();
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Fun>temp;  //与pendingFunctors进行swap，这样后面执行函数时只需使用temp即可，减小锁的粒度
    {
        MutexLockGuard m(mutex_);
        temp.swap(pendingFunctors);
    }
    for(const auto& func:temp)
    {
        func();
    }
}

void EventLoop::wakeReadCallBack()
{
    uint64_t wakeBytes = 0;

    auto n = read(wakefd_, &wakeBytes, sizeof(wakeBytes));
    if(n!=sizeof(wakeBytes))
    {
        LOG_ERROR<<"wakefd read "<<n<<" bytes instead of "<< sizeof(wakeBytes);
    }
}

void EventLoop::insertFd(int fd)
{
    fds[fd] = 0;
}

void EventLoop::eraseFd(int fd)
{
    fds.erase(fd);
}

int EventLoop::hbIncrease(int fd)
{
    return ++fds[fd];
}

void EventLoop::hbReset(int fd)
{
    fds[fd] = 0;
}

void EventLoop::hbReadCallBack()   //在io线程中执行心跳检测，如果放在主线程执行心跳检测，那么如果连接所在的io线程阻塞了，根本没有能力发出心跳包，而此时主线程代为发出心跳包就是“伪心跳”
{
    uint64_t tm;
    read(hbfd_,&tm, sizeof(tm));
    //const char* PING = "PING\n";

    for(auto connfd : fds)
    {
        int fd = connfd.first;
        //ssize_t writelen = write(fd,PING,strlen(PING));
        //assert(writelen==5);
        hbIncrease(fd);

        LOG_INFO<<"heart beaten for fd "<<fd<<" with count "<<connfd.second;
        if(connfd.second >= hblimit_)
        {
            hbCb(fd);
        }
    }
}

void EventLoop::setHbCloseCallBack(const hbCloseCallBack& cb)
{
    hbCb = cb;
}
