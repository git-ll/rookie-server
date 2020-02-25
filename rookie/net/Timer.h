

#ifndef ROOKIE_TIMERHEAP_H
#define ROOKIE_TIMERHEAP_H

#include <sys/time.h>
#include <queue>
#include "Channel.h"
#include "Util.h"
#include <unordered_map>

namespace rookie
{
    class Channel;
    class EventLoop;

    struct cmp
    {
        bool operator()(Channel* p, Channel* q){ return greater(p->timeout(),q->timeout());}
    };
    class Timer
    {
    public:
        Timer(EventLoop*);
        ~Timer();

        void addTimer(Channel*);       //添加一个定时器，如果已存在就覆盖
        void removeTimer(Channel*);    //删除一个定时器

        int getFirstTimeOut();        //获取当前定时器中最先超时的事件离现在还有多少毫秒
        void handleTimeOuts();         //将超时的Channels放到loop中
    private:
        timeval now;    //当前时间
        EventLoop* loop_;  //只负责使用EventLoop，不负责创建和销毁
        std::priority_queue<Channel*, std::vector<Channel*>, cmp>timer_queue;  //priority_queue没有提供删除结点的操作，用惰性删除的方法频繁检查损失效率，考虑自己实现一个小顶堆？
    };
}


#endif //ROOKIE_TIMERHEAP_H
