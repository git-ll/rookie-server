
#ifndef ROOKIE_TCPSERVER_H
#define ROOKIE_TCPSERVER_H

#include "../base/noncopyable.h"
#include <memory>
#include <unistd.h>
#include <netinet/in.h>
#include <unordered_map>
#include <map>

namespace rookie
{
    class EventLoop;
    class EventLoopThreadPool;
    class Channel;
    class Socket;
    class Connection;

    typedef std::shared_ptr<Connection> ConnectionPtr;
    typedef std::function<void(const ConnectionPtr& conn,const char* msg,ssize_t len)>MessageCb;
    typedef std::function<void(const ConnectionPtr& conn)>CloseCb;

    class TCPServer:noncopyable
    {
    public:
        TCPServer(const char* ip,uint16_t port,int threadNum = 4);
        TCPServer(uint16_t port,int threadNum = 4);
        ~TCPServer();

        void setMessageCallBack(const MessageCb& cb);

        void start();
    private:
        void acceptCallBack();
        void newConnection(int connfd,sockaddr_in& peer);
        void removeConnection(const ConnectionPtr& conn);
        void hbCloseCallBack(int fd);      //用于io线程心跳检测失效时调用该函数来销毁Connection实例
        std::unique_ptr<EventLoop>loop_;
        std::unique_ptr<EventLoopThreadPool>pool_;
        std::unique_ptr<Socket>acceptsock_;
        std::unique_ptr<Channel>acceptChannel_;
        std::unordered_map<int,ConnectionPtr>ConnectionMap;   //ConnectionMap的作用主要用于延长Connection的生命周期
        MessageCb messageCb;  //为所有连接指定消息处理函数
    };
}



#endif //ROOKIE_TCPSERVER_H
