

#include <functional>
#include <arpa/inet.h>
#include "TCPServer.h"
#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "LogOutput.h"
#include "Connection.h"
#include "Socket.h"
#include "Channel.h"
#include "Util.h"

using namespace rookie;

void defaultMessageCallBack(const ConnectionPtr& conn,const char* buf,ssize_t len)
{
    //time_t tm = time(nullptr) + 8*60*60;
    //struct tm t = {};
    //gmtime_r(&tm,&t);

    //printf("%04d%02d%02d %02d:%02d:%02d [%s:%d] : %s", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,conn->peerip(),conn->peerport(),buf);
    conn->connWrite(buf,len);
}

TCPServer::TCPServer(const char *ip, uint16_t port, int threadNum)
          :loop_(new EventLoop()),
           pool_(new EventLoopThreadPool(&*loop_,threadNum)),
           acceptsock_(new Socket(socket(AF_INET,SOCK_STREAM,0))),
           acceptChannel_(new Channel(acceptsock_->fd(),&*loop_)),
           messageCb(std::bind(&defaultMessageCallBack,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3))
{
    if(!loop_||!pool_||!acceptChannel_)
        LOG_FATAL<<__func__<<"allocate failed.";

    acceptsock_->setReuseAddr(true);
    acceptsock_->bind(ip,port);
    acceptsock_->setNonBlocking(true);
    acceptChannel_->enableChannel(EV_READ);
    acceptChannel_->setReadCallBack(std::bind(&TCPServer::acceptCallBack,this));
    LOG_INFO<<"TCPServer create.  ip : "<<ip<<"  port : "<<port<<"  accpect sock : "<<acceptsock_->fd();
    printf("server start...\n");
}

TCPServer::TCPServer(uint16_t port,int threadNum)
          :loop_(new EventLoop()),
           pool_(new EventLoopThreadPool(&*loop_,threadNum)),
           acceptsock_(new Socket(socket(AF_INET,SOCK_STREAM,0))),
           acceptChannel_(new Channel(acceptsock_->fd(),&*loop_)),
           messageCb(std::bind(&defaultMessageCallBack,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3))
{
    acceptsock_->setReuseAddr(true);
    acceptsock_->bind(port);
    acceptsock_->setNonBlocking(true);
    acceptChannel_->enableChannel(EV_READ);
    acceptChannel_->setReadCallBack(std::bind(&TCPServer::acceptCallBack,this));
    LOG_INFO<<"TCPServer create.  ip : 0.0.0.0  port : "<<port<<"  accpect sock : "<<acceptsock_->fd();
    printf("server start...\n");
}

TCPServer::~TCPServer()
{

}

void TCPServer::start()
{
    acceptsock_->listen();
    pool_->start();//threadpool每创建一个io线程都会调用startloop，而在startLoop中已经通过mutex和cond保证了子线程的eventLoop会先于主线程运行起来，因此最终子线程的loops肯定都先于主线程loop运行

    loop_->loop();
}

void TCPServer::acceptCallBack()
{
    sockaddr_in peeraddr = {};
    while(1)
    {
        int connfd = acceptsock_->accept(peeraddr);
        if(connfd == -1)
        {
            if(errno == EAGAIN)
            {
                LOG_DEBUG<<"accept all connecions.";
                break;
            }
            else if(errno == EINTR)continue;
            else
            {
                LOG_FATAL<<"accept error";
            }
        }
        if(setSocketNonBlocking(connfd) == -1)
        {
            LOG_FATAL<<"setSocketNonBlocking error";
        }
        newConnection(connfd,peeraddr);
        memset(&peeraddr,0,sizeof(sockaddr_in));
    }

    LOG_DEBUG<<"TCPServer::acceptCallBack() finished";
}

void TCPServer::newConnection(int connfd,sockaddr_in& peer)
{
    EventLoop* ioloop = pool_->nextLoop();   //如果线程池大小为0，那么ioloop就是主线程中的loop
    if(!ioloop->hbCb)ioloop->setHbCloseCallBack(std::bind(&TCPServer::hbCloseCallBack,this,std::placeholders::_1));
    ConnectionPtr conn = std::make_shared<Connection>(ioloop,connfd,peer);  //conn引用计数为1
    ConnectionMap[connfd] = conn;  //增加一次引用计数,至此conn的引用计数为2 当newConnection函数结束后，由于ConnectionMap的存在，使得conn的引用计数为1
    conn->setCloseCallBack(std::bind(&TCPServer::removeConnection,this,std::placeholders::_1));
    conn->setMessageCallBack(messageCb);

    //runInLoop会唤醒ioloop线程
    ioloop->runInLoop(std::bind(&Connection::connEstablished,conn));  //这里绑定时会对conn进行拷贝，这份拷贝会存在与pendingFunctors中，当处理完对应的func后，这份拷贝被析构
    //当该函数结束时，由于ConnectionMap的存在，conn的引用计数至少为1，如果还未调用pendingFunctor，那么引用计数为2，不过很快就会减少到1，ConnectionMap让conn保持存活
}

void TCPServer::setMessageCallBack(const MessageCb& cb)
{
    messageCb = cb;
}

void TCPServer::removeConnection(const ConnectionPtr& conn)   //当Connection需要关闭时就会调用这个函数，即Connection中的closeCb，传入shared_from_this，因此调用时Connection的引用计数为2
{

//    LOG_INFO<<"ConnectionMap erase.";
    //此时Connection的引用计数为1，但是当removeConnection结束后这一次引用计数就会减少
    conn->getLoop()->runInLoop([conn]{
        conn->channel()->disableChannel();
        conn->getLoop()->eraseFd(conn->fd());});
    ConnectionMap.erase(conn->fd());  //减少一次引用计数
    //使用runInloop的原因是因为，removeConnection是在handleChannel中处理的，runInloop可以让Connection的最后一次引用计数保存在channel所在的pendingFunctors中，而在执行pendingFunctors的时候，handleChannel必定已经执行结束，并且pendingFunctors执行结束时，最后一次引用计数也减少了从而导致Connection的销毁，此时销毁Connection就是安全的，因此调用runInLoop是有必要的！
}

void TCPServer::hbCloseCallBack(int fd)  //当某个连接心跳失效，就回调该函数来清除该连接
{
    LOG_INFO<<"heartbeat invalid.Ready to close fd "<<fd;
    removeConnection(ConnectionMap[fd]);
}
















