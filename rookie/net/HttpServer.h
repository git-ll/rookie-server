

#ifndef ROOKIE_HTTPSERVER_H
#define ROOKIE_HTTPSERVER_H

#include "TCPServer.h"
#include <mysql/mysql.h>


namespace rookie
{
    class httpRequest;
    class httpResponse;
    typedef std::function<void(httpRequest* request,httpResponse* response)>requestCb;    //传入请求报文，传出响应报文

    class HttpServer : noncopyable
    {
    public:
        HttpServer(const char* ip,uint16_t port);

        void start()
        {
            server.start();
        }
        void setRequestCallBack(const requestCb& cb);

    private:
        void OnMessage(const ConnectionPtr& conn,const char* msg,ssize_t len); //消息回调函数

        int parseHttpRequest(const char* msg,httpRequest* request);   //解析http报文,-1表示请求报文无效，0表示有效
        int findCRLF(const char* str);    //寻找CRLF的位置 ， -1表示没找到
        void parseRequestLine(httpRequest* request,const char* start,const char* end);   //解析请求行
        void encodeResponse(httpResponse *response,std::string& responseStr);   //根据response编码响应字符串

    private:
        TCPServer server;
        requestCb OnRequestCb;     //报文解析后，由OnRequestCb进行处理并制定响应报文
        std::string responseStr;
    };
}




#endif //ROOKIE_HTTPSERVER_H
