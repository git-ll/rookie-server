

#include "Epoll.h"
#include <sys/epoll.h>
#include <unistd.h>
#include "LogOutput.h"
#include "Channel.h"
#include "EventLoop.h"

using namespace rookie;

unsigned MAXEVENTS = 1024;

Epoll::Epoll(EventLoop* evloop)
      :epollfd_(epoll_create1(EPOLL_CLOEXEC)),
       ActiveEvents(MAXEVENTS),
       loop_(evloop)
{
    if(epollfd_ == -1)
        LOG_FATAL<<"Epoll::epoll_create1";
}

Epoll::~Epoll()
{
    close(epollfd_);
}

void Epoll::epoll_update(Channel* channel)
{
    int fd = channel->fd();
    epoll_event event={};
    memset(&event,0,sizeof(event));
    event.data.fd = fd;
    event.events = channel->events() | EPOLLET;    //ET模式
    ChannelMap[fd] = channel;
    if(!(channel->state() & EVSTATE_INSERT)) //如果未添加，就添加
    {
        LOG_DEBUG<<"Epoll::epoll_update : channel not exists,ready to add fd "<<fd;
        if(epoll_ctl(epollfd_,EPOLL_CTL_ADD,fd,&event)<0)
        {
            LOG_FATAL<<"Epoll::epoll_add"<<errno;
        }
        channel->setstate(channel->state() | EVSTATE_INSERT);
        LOG_DEBUG<<"channel added fd "<<fd;
        nChannel++;
    }
    else  //如果已经添加就修改
    {
        LOG_DEBUG<<"Epoll::epoll_update : channel exists,ready to modify with fd "<<fd;
        if(epoll_ctl(epollfd_,EPOLL_CTL_MOD,fd,&event)<0)
        {
            LOG_FATAL<<"Epoll::epoll_mod";
        }
        LOG_DEBUG<<"channel modified fd "<<fd;
    }
}

void Epoll::epoll_remove(Channel* channel)
{
    if(channel->state() & EVSTATE_INSERT)
    {
        if(epoll_ctl(epollfd_,EPOLL_CTL_DEL,channel->fd(),nullptr) < 0)
        {
            LOG_FATAL<<"Epoll::epoll_del";
        }
        //ChannleMap[channel->fd_] = nullptr;
        channel->setstate(EVSTATE_NONE);  //暂时不从ChannelMap中删除，只是设置channel的标志为NONE
        LOG_DEBUG<<"channel deleted from epoll with fd "<<channel->fd();
        nChannel--;
    }
    else
    {
        LOG_WARN<<"channel not exists ";
    }
}

void Epoll::epoll_dispatch(int timeout)
{
    int nevents = epoll_wait(epollfd_,&*ActiveEvents.begin(),MAXEVENTS,timeout);

    if(nevents < 0)
    {
        if(errno == EINTR)return;
        LOG_FATAL<<"epoll_wait";
    }
    else if(nevents == 0)
    {
        LOG_DEBUG<<"epoll_wait nothing happened";
    }
    else
    {
        for(int i = 0;i<nevents;i++)
        {
            auto p = ActiveEvents[i];
            LOG_DEBUG<<"active fd "<<p.data.fd;
            auto channel = ChannelMap[p.data.fd];
            channel->setrevents(p.events);
            loop_->activeChannel(channel);
        }
        if(nevents == MAXEVENTS)
        {
            MAXEVENTS*=2;
            ActiveEvents.resize(MAXEVENTS);
        }
        ActiveEvents.clear();
    }

}