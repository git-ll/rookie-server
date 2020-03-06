

#ifndef ROOKIE_EVENTLOOP_H
#define ROOKIE_EVENTLOOP_H

#include "../base/noncopyable.h"
#include "../base/Mutex.h"
#include "Epoll.h"
#include <memory>
#include <vector>
#include <map>

namespace rookie
{
    class Channel;
    class Epoll;
    class TimerWheel;
    class TCPServer;

    class EventLoop:noncopyable
    {

    public:
        typedef std::function<void()>Fun;
        typedef std::function<void(int)>hbCloseCallBack;
        EventLoop();
        ~EventLoop();

        void loop();
        void exit();

        void updateChannle(Channel* channel,int timeout);    //修改Channel，如果本身不存在，就转为添加
        void removeChannle(Channel* channel);    //删除Channel
        void activeChannel(Channel* channel);    //将Channel插入激活队列中
        void processChannels();      //处理激活的Channels
        void handleTimeOuts();       //处理定时器
        bool isLooping();
        void runInLoop(Fun func);      //非回调函数，但是需要在loop中执行的函数
        void doPendingFunctors();
        void insertFd(int fd);
        void eraseFd(int fd);
        void hbReset(int fd);
        int hbIncrease(int fd);
        void setHbCloseCallBack(const hbCloseCallBack&);
        pid_t threadid(){ return threadid_;}
    private:
        void wakeReadCallBack();
        void wakeupLoop();
        void hbReadCallBack();
        std::map<int,int>fds;    //当前Eventloop中存活的连接及其心跳计数
        Epoll poller_;
        std::vector<Channel*> ActiveChannels_;   //激活的事件集合  包括事件触发的以及超时激活的事件
        std::unique_ptr<TimerWheel>timer_;    //定时器  负责定时器的控制和释放
        int wakefd_;    //用于唤醒EventLoop
        int hbfd_;     //用于心跳包定时的fd
        int hbcycle_;  //心跳检测周期 (秒)
        int hblimit_;  //心跳失效次数
        std::unique_ptr<Channel>wakeChannel_;//与wakefd绑定
        std::unique_ptr<Channel>hbChannel_; //心跳定时Channel，与hbfd_绑定
        hbCloseCallBack hbCb;
        std::vector<Fun>pendingFunctors;    //需要在loop中执行而未执行的函数
        bool looping_;
        bool stop_;
        bool iswaiting_;
        const pid_t threadid_;
        MutexLock mutex_;
        TCPServer* server_;
        friend Epoll;
        friend TCPServer;
    };



}




#endif //ROOKIE_EVENTLOOP_H
