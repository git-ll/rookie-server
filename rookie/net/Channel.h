

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
#define EVSTATE_ALL EVSTATE_NONE|EVSTATE_INSERT|EVSTATE_TIMEOUT

namespace rookie
{
    class EventLoop;
    class Epoll;
    class Channel
    {
    public:
        Channel(int fd,EventLoop* evloop);
        ~Channel() = default;

        typedef std::function<void()>CallBack;

        inline void setReadCallBack(const CallBack& r){ ReadCallBack_ = r;}
        inline void setWriteCallBack(const CallBack& w){ WriteCallBack_ = w;}
        inline void setErrorCallBack(const CallBack& e){ ErrorCallBack_ = e;}
        inline void setCloseCallBack(const CallBack& c){ CloseCallBack_ = c;}

        void enableChannel(uint32_t type ,int timeout = -1);//支持EV_READ/EV_WRTE/EV_TIMEOUT    timeout为秒计
        void disableChannel();  //关闭channel
        void handleChannel();   //处理channel
        const char* eventsToString(int events);

        int fd() { return fd_;}
        inline uint32_t events() { return events_;}    //返回Channel感兴趣的事件
        inline uint32_t revents(){ return revents_;}   //返回激活的事件
        inline void setevents(uint32_t ev){ events_ = ev;}    //设置感兴趣的事件
        inline void setrevents(uint32_t rv) { revents_ = rv;}    //设置激活的事件
        inline int state() { return state_;}    //获取channel的状态
        inline timeval timeout() { return timeout_;}
        inline void setstate(int state) { state_ = state;}   //设置channel的状态
        inline void setpriority(int pri) { pri_ = pri;}   //设置Channel的优先级
        inline int timerslot() { return tmslot_;}  //获取该channel在定时器中的位置
        inline void settimerslot(int slot) { tmslot_ = slot;}  //设置在定时器中的位置
    private:
        int fd_;        //相应的文件描述符
        uint32_t events_;    //感兴趣的事件
        uint32_t revents_;    //发生的事件
        EventLoop* evloop_;   //所在的EventLoop
        timeval timeout_;    //超时时刻
        int pri_;       //Channel优先级
        int state_;     //Channel的状态
        int tmslot_;     //对于非超时Channel，index_为-1，对于超时Channel，index为其在定时器中的索引

        CallBack ReadCallBack_;
        CallBack WriteCallBack_;
        CallBack ErrorCallBack_;
        CallBack CloseCallBack_;
    };
}


#endif //ROOKIE_CHANNEL_H
