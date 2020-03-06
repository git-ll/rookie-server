

#ifndef ROOKIE_TIMERWHEEL_H
#define ROOKIE_TIMERWHEEL_H

#include "./noncopyable.h"
#include "EventLoop.h"

namespace rookie
{
    class EventLoop;
    class Channel;
    class TimerWheel;
    class TimerNode
    {

    public:
        TimerNode(Channel* channel,int timeout);
        ~TimerNode() = default;

        TimerNode* next;
        TimerNode* prev;
    private:
        Channel* channel_;
        int slot_;    //该定时器位于哪个槽
        int rotation_;   //表示这个定时器还需要多少圈会触发

        friend TimerWheel;
    };
    class TimerWheel:noncopyable
    {
    public:
        TimerWheel(EventLoop* evloop,int size = 60,int tick = 1);
        ~TimerWheel();

        void addTimer(Channel* channel,int timeout);
        void removeTimer(Channel* channel);
        void handleTimeOut();
        void tick();    //时间轮走一步
        void expand();  //对时间轮进行扩容

    private:
        EventLoop* loop_;
        int size_;   //槽数
        int timercnt;    //时间轮中的定时器数量
        int tick_ ;   //1s转一个槽
        int expandlimit_;    //扩容阈值
        float expandfactor_;   //扩容因子
        TimerNode** slots;   //轮子，串联起所有的slot，每个slot都是一个Timer的链表
        int curslot_;    //当前指向的槽
        time_t lasttime_;  //记录上一次转动的时间
    };

}



#endif //ROOKIE_TIMERWHEEL_H
