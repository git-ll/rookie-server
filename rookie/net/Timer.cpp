/*****************************
 *
 *     @Name TimerHeap.cpp
 *
 *     @Date 19-11-29
 *
 *     @Author Liu Lei
 * 
 *     @Email  liulei805@qq.com
 *
 *****************************/

#include <cstring>
#include "Timer.h"
#include "EventLoop.h"
#include "../base/LogOutput.h"
#include "Util.h"

using namespace rookie;


Timer::Timer(EventLoop * loop)
      :loop_(loop)
{

}

Timer::~Timer()
{

}

int Timer::getFirstTimeOut()
{
    int res = -1;
    if(!timer_queue.empty()) //如果定时堆是空的，就说明没有超时事件，epoll_wait应该一直等待
    {
        auto tmp = timer_queue.top();

        while(!(tmp->state_ & EVSTATE_TIMEOUT))   //如果堆顶的元素没有EVSTATE_TIMEOUT标记，就看下一个
        {
            timer_queue.pop();
            if(timer_queue.empty())return res;
            tmp = timer_queue.top();
        }
        gettimeofday(&now,nullptr);
        timeval dif = timesub(tmp->timeout_,now);  //计算最先超时的event的超时时刻和当前的时间差，作为epoll_wait的超时时长

        //res = dif.tv_sec*1000+dif.tv_usec/1000;
        //如果直接用usec/1000实际是把微秒向下取整了，按照这个时间epoll_wait返回时可能未超时，比如说本身还有4.5ms超时，如果向下取整就变成了4ms，
        // epoll_wait超时4ms返回，可能由于还没有达到4.5ms而导致epoll_wait超时，因此应当让epoll_wait超时等待5ms来避免
        // 会导致epoll_wait空跑，因此应该把微秒向上取毫秒，

        res = dif.tv_sec*1000+(dif.tv_usec+999)/1000;
        if(res < 0)res = 0;  //小于0说明已经超时，此时就应该让epoll_wait立刻返回
    }
    return res;
}

void Timer::handleTimeOuts()
{
    gettimeofday(&now,nullptr);
    int cnt = 0;
    while(!timer_queue.empty())
    {
        auto tmp = timer_queue.top();
        if(!(tmp->state_ & EVSTATE_TIMEOUT))   //如果channel本身已经被删除了，就直接从定时器中删除掉
        {
            timer_queue.pop();
            continue;
        }
        if(greater(tmp->timeout_,now))break; //如果当前元素还未超时，那么后面的元素都不会超时，就结束遍历
        tmp->revents_ = tmp->events_ | EV_TIMEOUT;   //由于是超时而激活的，那么就将其激活类型直接设置为感兴趣的事件类型
        loop_->activeChannel(tmp);
        timer_queue.pop();
        tmp->state_ &= ~EVSTATE_TIMEOUT;
        cnt++;
    }
    LOG_DEBUG<<cnt<<" events timeout active.";
}

void Timer::addTimer(Channel* channel)
{
    if(channel->state_ & EVSTATE_TIMEOUT)
    {
        LOG_WARN<<"Timer::addTimer(Channel* channel) : channel has already in timer_queue";
        return;
    }
    channel->state_ |= EVSTATE_TIMEOUT;   //EVSTATE_TIMEOUT表示channel位于定时器中
    timer_queue.push(channel);
}

void Timer::removeTimer(Channel* channel)
{
    channel->state_ |= ~EVSTATE_TIMEOUT;   //表示channel不再位于定时器中
}



