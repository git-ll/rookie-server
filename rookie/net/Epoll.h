

#ifndef ROOKIE_EPOLL_H
#define ROOKIE_EPOLL_H

#include <sys/epoll.h>
#include <vector>
#include <unordered_map>
#include "../base/noncopyable.h"

namespace rookie
{
    class EventLoop;
    class Channel;

    class Epoll:noncopyable
    {
    public:
        Epoll(EventLoop*);
        ~Epoll();

        void epoll_update(Channel*);  //处理add和mod两种情况
        void epoll_remove(Channel*);
        void epoll_dispatch(int timeout);

    private:
        int epollfd_;
        int nChannel;   //epoll中监听的channel个数
        std::vector<epoll_event>ActiveEvents; //存放epoll_wait返回的激活的事件
        std::unordered_map<int,Channel*> ChannelMap;  //存放所有添加到epoll中的channel及其状态
        EventLoop* loop_;   //只负责使用EventLoop，不负责创建和销毁
    };
}



#endif //ROOKIE_EPOLL_H
