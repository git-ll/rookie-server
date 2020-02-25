

#ifndef ROOKIE_SOCKET_H
#define ROOKIE_SOCKET_H

#include <unistd.h>
#include <string>
#include <netinet/in.h>
#include "../base/noncopyable.h"
#include "../base/LogOutput.h"

namespace rookie
{
    using std::string;

    class Connection;
    class Socket : noncopyable
    {
    public:

        Socket(int fd);
        ~Socket(){ close(sockfd_); LOG_INFO<<"fd "<<sockfd_<<" close.";};


        int fd(){ return sockfd_;}
        void bind(const char* ip,uint16_t port);
        void bind(uint16_t port);
        void bind(sockaddr_in&);

        void listen(int maxConnect = 2048);

        void connect(sockaddr_in&);
        void connect(const char* ip,uint16_t port);

        int accept(sockaddr_in& peeraddr);

        void setNonBlocking(bool);
        void setTcpNoDelay(bool);   //关闭Nagle
        void setReuseAddr(bool);    //
        void setReusePort(bool);
        void setKeepAlive(bool);



        const sockaddr_in& getSockAddr() const { return addr_;}
        string Ip()const { return IPString_;}
        int Port() const { return port_;}
        int Family() const { return addr_.sin_family;}

    private:
        const int sockfd_;
        int peersockfd_;
        struct sockaddr_in addr_;
        struct sockaddr_in peeraddr_;
        char* IPString_;
        char* peerIPString_;
        uint16_t peerport_;
        uint16_t port_;

        bool connecting;
    };
}



#endif //ROOKIE_SOCKET_H
