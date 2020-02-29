

#include <arpa/inet.h>
#include "Connection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "fcntl.h"

using namespace rookie;

Connection::Connection(EventLoop* ioloop,int connfd, sockaddr_in peer)
               :loop_(ioloop),
                connfd_(connfd),
                peeraddr_(peer),
                peerport_(ntohs(peer.sin_port)),
                peerip_(inet_ntoa(peer.sin_addr)),
                channel_(new Channel(connfd,ioloop)),
                isConnect_(true)  //Connection实例创建时都认为是已经连接上了
{
    ip_ = inet_ntoa(peer.sin_addr);
    port_ = ntohs(peer.sin_port);
    channel_->setReadCallBack(std::bind(&Connection::connRead,this));
    LOG_INFO<<"new Connection ["<<ip_<<":"<<port_<<"] of fd "<<fd();
}

Connection::~Connection()
{
    LOG_INFO<<"Connection ["<<ip_<<":"<<port_<<"] of fd "<<fd()<<" exit.";
    close(connfd_);
}

void Connection::connEstablished()
{
    channel_->enableChannel(EV_READ);
    loop_->insertFd(connfd_);
    LOG_DEBUG<<"new connection established with fd "<<connfd_<<" for ["<<peerip_<<":"<<peerport_<<"]";
}

void Connection::connRead()
{
    char rdbuf[4096];
    int rdbytes = 0;
    while(1)
    {
        ssize_t n = read(connfd_,rdbuf+rdbytes,4096);
        loop_->hbReset(connfd_);

        if(n == 0)  //如果没有及时关闭fd，那么就会反复触发可读事件(LT)，如果是ET模式，就不会
        {
            //关闭连接,并在runinloop中销毁Connection实例
            LOG_DEBUG<<"read 0 bytes from fd "<<fd();
            connClose();
            return;
        }
        else if(n<0)
        {
            if(errno == EAGAIN)
            {
                break;
            }
            else if(errno == EINTR)
            {
                continue;
            }
            connError();
            return;
        }
        rdbytes+=n;
        if(rdbytes >= 4096)
        {
            rdbytes = 4095;
            break;
        }
    }
    rdbuf[rdbytes] = '\0';
    /*if(strcmp(rdbuf,"PANG") == 0)
    {
        LOG_INFO<<"heartbeat response!";
        loop_->hbReset(connfd_);   //Connection的读回调必定是在io线程中执行的，因此不用把hbReset放到pendingfunctors中
        return;
    }*/
    messageCb(shared_from_this(),rdbuf,rdbytes);
}

void Connection::connWrite(const char* msg,size_t len)
{
    channel_->enableChannel(EV_READ|EV_WRITE);
    channel_->setWriteCallBack(std::bind(&Connection::sendMsg,this,msg,len));
}

void Connection::sendMsg(const char* msg,size_t len)
{
    channel_->enableChannel(EV_READ);  //取消写事件监听，恢复监听读事件
    ssize_t writelen = 0;
    while(writelen<len)
    {
        ssize_t  n = write(connfd_,msg,len);
        if(n<0)
        {
            if(errno == EAGAIN)
            {
                break;
            }
            else if(errno == EINTR)
            {
                continue;
            }
            connError();
            return;
        }
        else
        {
            writelen+=n;
        }
    }
}

void Connection::connClose()
{
    isConnect_ = false;

    closeCb(shared_from_this());//调用closeCb时会增加一次引用计数  由于前面ConnectionMap的存在，因此shared_from_this能够成功
    //****connRead函数必定是在handleChannel中调用的，因此可以让Connection的销毁放到pendingFunctors中，这样就能保证Connection的销毁在handleChannel之后
    //****Connection的销毁应该有：处理ConnectionMap，销毁Channle以及关闭Socket，用shared_from_this可以让pendingFunctors执行结束后Connection自行销毁。
}

void Connection::connError()
{
    int err = 0;
    socklen_t len = sizeof(err);
    if(::getsockopt(connfd_, SOL_SOCKET, SO_ERROR, &err, &len) < 0)
    {
        LOG_ERROR<<"getsockopt error with fd "<<connfd_;
    }
    else
    {
        LOG_ERROR << "Connection::handleError [" << ip_ << ":" << port_<<"] - SO_ERROR = " << err << " " << strerror(err);
    }
}

void Connection::setCloseCallBack(const CloseCb& cb)
{
    closeCb = cb;
}

void Connection::setMessageCallBack(const MessageCb& cb)
{
    messageCb = cb;
}
