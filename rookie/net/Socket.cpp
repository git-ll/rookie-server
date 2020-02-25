

#include "Socket.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include "LogOutput.h"
#include <netinet/tcp.h>
#include <fcntl.h>
#include "Connection.h"

using namespace rookie;

Socket::Socket(int fd)
       :sockfd_(fd),
        connecting(false),
        peersockfd_(-1),
        port_(0),
        peerport_(0)
{
    memset(&addr_,0,sizeof addr_);
    memset(&peeraddr_,0,sizeof peeraddr_);
    if(sockfd_ <0)
        LOG_FATAL<<"Socket fd is illegal";
    LOG_DEBUG<<"Socket create fd "<<sockfd_;
}

void Socket::bind(const char* ip,uint16_t port)
{
    sockaddr_in tmp ;
    memset(&tmp,0,sizeof(tmp));
    tmp.sin_addr.s_addr = inet_addr(ip);
    tmp.sin_port = htons(port);
    tmp.sin_family = AF_INET;
    bind(tmp);
}

void Socket::bind(uint16_t port)
{
    sockaddr_in tmp ;
    memset(&tmp,0,sizeof(tmp));
    tmp.sin_addr.s_addr = INADDR_ANY;
    tmp.sin_port = htons(port);
    tmp.sin_family = AF_INET;
    bind(tmp);
}

void Socket::bind(sockaddr_in& addr)
{
    addr_ = addr;
    IPString_ = inet_ntoa(addr_.sin_addr);
    port_ = ntohs(addr_.sin_port);
    if(::bind(sockfd_,(sockaddr *)&addr,sizeof addr) < 0)
    {
        LOG_FATAL<<"bind failed with "<<IPString_<<':'<<port_;
        return;
    }
    LOG_DEBUG<<"bind success:"<<IPString_<<':'<<port_;
}

void Socket::listen(int maxConnect)
{
    if(::listen(sockfd_,maxConnect) < 0)
    {
        LOG_FATAL<<"bind failed";
        return;
    }
    LOG_INFO<<IPString_<<':'<<port_<<" listening...";
}

void Socket::connect(sockaddr_in& peeraddr)
{
    if(::connect(sockfd_,(sockaddr *)&peeraddr,sizeof peeraddr)<0)
    {
        LOG_ERROR<<"connect failed";
        return;
    }
    peeraddr_ = peeraddr;
    peerIPString_ = inet_ntoa(peeraddr_.sin_addr);
    peerport_ = ntohs(peeraddr_.sin_port);
    LOG_DEBUG<<"连接成功:"<<peerIPString_<<':'<<peerport_;
}

void Socket::connect(const char* ip,uint16_t port)
{
    sockaddr_in tmp ;
    memset(&tmp,0,sizeof(tmp));
    tmp.sin_family = AF_INET;
    tmp.sin_addr.s_addr = inet_addr(ip);
    tmp.sin_port = ntohs(port);
    connect(tmp);
}

int Socket::accept(sockaddr_in& peeraddr)
{
    socklen_t len = sizeof(peeraddr);
    int peersockfd = ::accept(sockfd_,(sockaddr *)&peeraddr,&len);
    return peersockfd;
}

void Socket::setNonBlocking(bool on)
{
    int flag = fcntl(sockfd_, F_GETFL, 0);

    if(on)
    {
        flag |= O_NONBLOCK;
    }
    else
    {
        flag &= ~O_NONBLOCK;
    }
    if(fcntl(sockfd_, F_SETFL, flag) == -1)
        LOG_FATAL<<"Socket::setNonBlocking";
}

void Socket::setTcpNoDelay(bool on)
{
    int ops = on?1:0;
    int ret = ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &ops, sizeof(ops));

    if (ret < 0 && on)
    {
        LOG_FATAL << "SO_REUSEPORT failed.";
    }
}

void Socket::setReuseAddr(bool on)
{
    int ops = on?1:0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &ops, sizeof(ops));

    if (ret < 0 && on)
    {
        LOG_FATAL << "SO_REUSEADDR failed.";
    }
}

void Socket::setReusePort(bool on)
{
    int ops = on?1:0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &ops, sizeof(ops));
    if (ret < 0 && on)
    {
        LOG_FATAL << "SO_REUSEPORT failed.";
    }
}

void Socket::setKeepAlive(bool on)
{
    int ops = on?1:0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &ops, sizeof(ops));
    if (ret < 0 && on)
    {
        LOG_FATAL << "SO_REUSEPORT failed.";
    }
}


