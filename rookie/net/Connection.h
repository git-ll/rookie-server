

#ifndef ROOKIE_CONNECTIONINFO_H
#define ROOKIE_CONNECTIONINFO_H

#include "../base/noncopyable.h"
#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include "Socket.h"
#include "TCPServer.h"
#include <vector>

namespace rookie
{
    class Channel;
    class EventLoop;

    //Connection将会作为参数传递给外部函数对象MessageCallBack等等，因此Connection的生命周期必须比MessageCallBack更长，
    //因此可以用shared_from_this来将当前的Connection对象转换为一个shared_ptr对象传入MessageCallBack，这样Connection对象的生命周期就会延长而不会在MessageCallBack调用时出错了
    class Connection:noncopyable,public std::enable_shared_from_this<Connection>   //每一条连接都对应一个Connection实例，负责该条连接的读写
    {
    public:
        Connection(EventLoop* ioloop,int connfd,sockaddr_in peer);
        ~Connection() ;

        void connRead();    //连接有数据可读时触发
        void connWrite(const char* msg,size_t len);   //向EventLoop注册可写事件
        void connClose();   //连接关闭时触发
        void connError();   //连接出错时触发

        void setMessageCallBack(const MessageCb& cb);
        void setCloseCallBack(const CloseCb& cb);
        void connEstablished();   //有新连接后调用该函数，在acceptChannel的回调函数中调用

        int fd(){ return connfd_;}
        EventLoop* getLoop() { return loop_;}
        Channel* channel(){ return channel_.get();}
        const char* peerip() { return peerip_;}
        uint16_t peerport() { return peerport_; }

    private:
        void sendMsg(const char* msg,size_t len);  //可写事件触发时的回调函数

        int connfd_;
        bool isConnect_;      //connected or closed
        sockaddr_in peeraddr_;
        const char* peerip_;
        uint16_t peerport_;
        uint16_t port_;
        const char* ip_;
        EventLoop* loop_;
        std::unique_ptr<Channel>channel_;     //每一条连接都对应一个Channel，其生命周期由Connection控制  注意，unique_ptr并不一定是好的办法，可能Channel正在执行handleChannel，如果销毁了Channel就会出错
        std::vector<char>rdbuf_;
        MessageCb messageCb;     //处理接收消息的函数
        CloseCb closeCb;   //连接断开时的处理函数
    };
}




#endif //ROOKIE_CONNECTIONINFO_H
