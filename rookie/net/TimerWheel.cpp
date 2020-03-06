

#include "TimerWheel.h"
#include "LogOutput.h"
#include "Channel.h"
#include <time.h>
#include <iostream>

using namespace rookie;

TimerNode::TimerNode(Channel* channel,int rotation)
      :channel_(channel),prev(nullptr),next(nullptr)
{

}

TimerWheel::TimerWheel(rookie::EventLoop *evloop, int size, int tick)
           :loop_(evloop),size_(size),tick_(tick),lasttime_(time(nullptr)),curslot_(0),
            expandfactor_(0.75),expandlimit_(static_cast<int>(expandfactor_*size_)),timercnt(0)
{
    slots = new TimerNode*[size];  //创建size个槽
    for(int i=0;i<size;i++)slots[i] = nullptr;
}

TimerWheel::~TimerWheel()
{
    for( int i = 0; i < size_; ++i )
    {
        auto tmp = slots[i];
        while( tmp )
        {
            slots[i] = tmp->next;
            delete tmp;
            tmp = slots[i];
        }
    }
    LOG_INFO<<"TimerWheel Distroy";
}

void TimerWheel::addTimer(Channel *channel,int timeout)
{
    if(timeout < 0)
    {
        LOG_WARN<<"fd "<<channel->fd()<<"： timeout should more than 0 .";
        return;
    }
    int ticks = timeout/tick_;  //计算需要转动几个槽
    int rotation = ticks / size_;   //计算需要转几圈
    int ts = (curslot_ + ticks)%size_;   //计算在哪个槽超时
    auto timer = new TimerNode(channel,rotation);
    timer->slot_ = ts;
    timer->rotation_ = rotation;

    if(!slots[ts])   //如果这个槽里面没有定时器
    {
        slots[ts] = timer;
    }
    else   //头插法
    {
        timer->next = slots[ts];
        slots[ts]->prev = timer;
        slots[ts] = timer;
    }
    channel->setstate(channel->state() | EVSTATE_TIMEOUT) ;
    channel->settimerslot(ts);
   // LOG_INFO<<"Add new Timer to slot "<<ts<<" of rotation "<<rotation<<" add current slot is "<<curslot_;

    if(timercnt++ >= expandlimit_)
    {
        expand();
    }
}

void TimerWheel::removeTimer(Channel *channel)
{
    int slot = channel->timerslot();
    auto curTimer = slots[slot];
    while(curTimer)
    {
        if(curTimer->channel_ == channel)  //找到channel对应的那个Timer
        {
            if(curTimer == slots[slot])  //如果这个Timer就是头结点
            {
                slots[slot] = curTimer->next;  //指向下一个Timer
                if(slots[slot])
                {
                    slots[slot]->prev = nullptr;   //prev指针置空
                }

            }
            else   //非头部结点激活，删除即可
            {
                curTimer->prev->next = curTimer->next;
                if(curTimer->next)
                {
                    curTimer->next->prev = curTimer->prev;
                }
            }
            delete curTimer;  //删除当前Timer
            channel->setstate(channel->state() | ~EVSTATE_TIMEOUT);  //表示channel不再位于定时器中
            timercnt--;
            LOG_ERROR<<"fd : "<<channel->fd()<<" : channel delete in Timerwheel";
            return;
        }
        else
        {
            curTimer = curTimer->next;
        }
    }
    LOG_ERROR<<"fd : "<<channel->fd()<<" : channel not exist in Timerwheel";
}

void TimerWheel::tick()  //遍历当前slot，将超时的Timer对应的channel激活
{
    LOG_DEBUG<<"Timer tick and current slot is "<<curslot_;
    auto curTimer = slots[curslot_];
    while(curTimer)
    {
        if( curTimer->rotation_ > 0 )
        {
            curTimer->rotation_--;
            curTimer = curTimer->next;
        }
        else
        {
            auto channel = curTimer->channel_;
            channel->setrevents(channel->events() | EV_TIMEOUT);  //设置激活事件类型为全部
            channel->setstate(channel->state() & ~EVSTATE_TIMEOUT);  //表示channel不再位于定时器中

            loop_->activeChannel(channel);   //将channel激活
            if( curTimer == slots[curslot_] )    //如果超时的Timer正好位于头部
            {
                slots[curslot_] = curTimer->next;  //指向下一个Timer
                delete curTimer;  //删除当前Timer
                if(slots[curslot_])
                {
                    slots[curslot_]->prev = nullptr;   //prev指针置空
                }
                curTimer = slots[curslot_];
            }
            else   //非头部结点激活，删除即可
            {
                curTimer->prev->next = curTimer->next;
                if(curTimer->next)
                {
                    curTimer->next->prev = curTimer->prev;
                }
                auto tmp2 = curTimer->next;
                delete curTimer;
                curTimer = tmp2;
            }

        }
    }
    if(++curslot_ == size_)curslot_ = 0; //向前走一格

}

void TimerWheel::handleTimeOut()
{
    time_t now = time(nullptr);
    time_t spend = now-lasttime_;
    lasttime_ = now;
    for(int i=0;i<spend;i++)tick();    //过了多少秒就转多少次
}

void TimerWheel::expand()  //时间轮扩容函数
{
    int newsize = size_;
    while(0.75*newsize <= timercnt)newsize*=2;  //加倍扩容
    LOG_INFO<<"Expand  Size : "<<size_<<" to NewSize : "<<newsize<<" while TimerCount : "<<timercnt;

    TimerNode** newSlots = new TimerNode*[newsize];

    for(int i=0;i<newsize;i++)newSlots[i] = nullptr;

    for(int i=0;i<size_;i++)
    {
        TimerNode* curTimer = slots[i];

        while(curTimer)
        {
            int newslot = (curTimer->rotation_*size_+curTimer->slot_)%newsize;   //计算在新的时间轮中的位置
            int newrotation = (curTimer->rotation_*size_+curTimer->slot_)/newsize;
            TimerNode* next = curTimer->next;   //取得下一个Timer
            LOG_INFO<<"rotation : "<<curTimer->rotation_<<" ---> "<<newrotation<<"   slot : "<<curTimer->slot_<<" ---> "<<newslot;

            curTimer->slot_ = newslot;
            curTimer->rotation_ = newrotation;

            if(!newSlots[newslot])   //如果新的槽里面没有定时器
            {
                LOG_DEBUG<<"Slot is empty!Add new Timer to new TimerWheel of slot "<<newslot<<" of rotation "<<newrotation;
                newSlots[newslot] = curTimer;
                curTimer->next = nullptr;
            }
            else   //头插法
            {
                curTimer->next = newSlots[newslot];
                newSlots[newslot]->prev = curTimer;
                newSlots[newslot] = curTimer;
            }
            curTimer = next;  //到下一个Timer
        }
    }
    delete slots;  //释放原来的时间轮
    slots = newSlots;   //更新为新的时间轮
    curslot_ = 0;
    size_ = newsize;
    expandlimit_ = static_cast<int>(newsize*expandfactor_);
    LOG_INFO<<"expand finished";
}
