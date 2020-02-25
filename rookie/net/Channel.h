

#ifndef ROOKIE_CHANNEL_H
#define ROOKIE_CHANNEL_H

#include <functional>
#include <sys/epoll.h>

//type
/*
 * EV_READ表示Channel监听读事件
 * EV_WRITE表示Channel监听写事件
 * EV_SIGNAL表示Channel监听信号事件
 * EV_TIMEOUT表示Channel监听超时事件
 */
#define EV_NONE    0x00
#define EV_READ    EPOLLIN   //0x01
#define EV_WRITE   EPOLLOUT  //0x04
#define EV_SIGNAL  0x02
#define EV_TIMEOUT 0X1000
#define EV_ALL     EV_NONE|EV_READ|EV_WRITE|EV_SIGNAL|EV_TIMEOUT

//state
/*
 * EVSTATE_NONE表示未添加到Epoll中
 * EVSTATE_INSERT表示已经添加到Epoll中
 * EVSTATE_TIMEOUT表示位于定时堆中
 * */
#define EVSTATE_NONE    0x00
#define EVSTATE_INSERT  0x01
#define EVSTATE_TIMEOUT 0x02

namespace rookie
{
    class EventLoop;
    class Epoll;
    class Timer;
    class Channel
    {
    public:
        Channel(int fd,EventLoop* evloop);
        ~Channel() = default;

        typedef std::function<void()>CallBack;

        void setReadCallBack(const CallBack& r){ ReadCallBack_ = r;}
        void setWriteCallBack(const CallBack& w){ WriteCallBack_ = w;}
        void setErrorCallBack(const CallBack& e){ ErrorCallBack_ = e;}
        void setCloseCallBack(const CallBack& c){ CloseCallBack_ = c;}

        void enableChannel(uint32_t flags ,int timeout = -1);//timeout为毫秒计
        void disableChannel();

        void handleChannel();
        const char* eventsToString(int events);

        int fd(){ return fd_;}

        uint32_t events() { return events_;}     //返回Channel感兴趣的事件
        uint32_t revents() { return revents_;}   //返回激活的事件
        int state(){ return state_;}    //获取channel的状态
        timeval timeout(){ return timeout_;}
        void setstate(int state){ state_ = state;}   //设置channel的状态
        void setpriority(int pri){ pri_ = pri;}   //设置Channel的优先级

    private:
        int fd_;        //相应的文件描述符
        uint32_t events_;    //感兴趣的事件
        uint32_t revents_;    //发生的事件
        EventLoop* evloop_;   //所在的EventLoop
        timeval timeout_;    //超时时刻，微秒计
        int pri_;       //Channel优先级
        int state_;     //Channel的状态
        int index_;     //对于非超时Channel，index_为-1，对于超时Channel，index为其在定时器中的索引

        CallBack ReadCallBack_;
        CallBack WriteCallBack_;
        CallBack ErrorCallBack_;
        CallBack CloseCallBack_;

        friend Epoll;
        friend EventLoop;
        friend Timer;
    };
}


#endif //ROOKIE_CHANNEL_H
