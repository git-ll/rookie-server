

#include "HttpServer.h"
#include "HttpData.h"
#include "Connection.h"
#include <sys/stat.h>
#include <fcntl.h>
#include "Util.h"

using namespace rookie;

/*void defaultOnRequest(httpRequest *request, httpResponse *response)   //request已经包含了Http的请求信息，response为响应信息
{
    if(request->requestGET())
    {
        if (request->isURL("/") || request->isURL("/index.html"))
        {
            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("text/html");
            response->setBodyFromFile("resource/index.html");
        }
        else if (request->isURL("/favicon.ico"))
        {
            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("image/ico");
            response->setBodyFromFile("dist/images/favicon.ico");
        }
        else if (request->isURL("/hello"))
        {
            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("text/plain");
            response->setBody("HELLO!");
        }
        else if (request->isURL("/login.js"))
        {
            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("text/javascript");
            response->setBodyFromFile("resource/login.js");
        }
        else if (request->isURL("/login.css"))
        {
            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("text/css");
            response->setBodyFromFile("resource/login.css");
        }
        else if (request->isURL("/register.html"))
        {
            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("text/html");
            response->setBodyFromFile("resource/register.html");
        }
        else
        {
            response->setNotFound();
        }

    }
    else if(request->requestPOST())
    {
        if (request->isURL("/user/loginin"))  //登录
        {
            char username[32];
            char password[32];
            sscanf(UrlDecode(request->body()).c_str(),"username=%s password=%s",username,password);

            //数据库验证....
            LOG_INFO<<"用户名："<<username<<"   密码："<<password;

            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("text/html");
            response->setBody("hello!");
        }
        else if(request->isURL("/user/register"))
        {
            char username[32] = {0};
            char nickname[32] = {0};
            char password[32] = {0};
            char phonenum[11] = {0};

            sscanf(UrlDecode(request->body()).c_str(),"nickname=%s username=%s password=%s phonenum=%s",nickname,username,password,phonenum);

            //数据库插入....
            LOG_INFO<<"昵称："<<nickname<<"   用户名："<<username<<"   密码："<<password<<"   手机号："<<phonenum;

            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("text/html");
            response->setBody("ok");

        }


    }

}*/

/*void printRequest(httpRequest* request)
{
    printf("PARSE RESULT:\n");
    printf("[METHOD] %s\n",request->method_);
    printf("[URL] %s\n",request->url_);
    printf("[VERSION] %s\n",request->version_);

    for(auto &i:request->requestHeaders_)
    {
        printf("[HEADER] %s [VALUE] %s\n",i.first.c_str(),i.second.c_str());
    }

    printf("[BODY] %s\n",request->body_.c_str());

}*/

void defaultOnRequest(httpRequest *request, httpResponse *response)   //request已经包含了Http的请求信息，response为响应信息
{
    if(request->requestGET())
    {
        if (request->isURL("/") || request->isURL("/index.html"))
        {
            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("text/html");
            response->setBodyFromFile("resource/home.html");
        }
        else if (request->isURL("/favicon.ico"))
        {
            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("image/ico");
            response->setBodyFromFile("resource/favicon.ico");
        }
        else
        {
            response->setNotFound("404 NOT FOUND");
        }
    }
}

HttpServer::HttpServer(const char* ip,uint16_t port)
        :server(ip,port,4),
         OnRequestCb(std::bind(&defaultOnRequest,std::placeholders::_1,std::placeholders::_2))
{
    server.setMessageCallBack(std::bind(&HttpServer::OnMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
}


void HttpServer::OnMessage(const rookie::ConnectionPtr &conn, const char *msg, ssize_t len) //服务端收到数据时调用该函数
{
    httpRequest request;    //Http请求
    httpResponse response;  //Http响应
    if(parseHttpRequest(msg,&request) == -1) //将收到的数据解析为Http请求格式
    {
        //请求无效
        responseStr = "HTTP/1.1 400 Bad Request\r\nContent-Length: 11\r\nConnection: close\r\n\r\nBad Request\r";
    }
    else
    {
        OnRequestCb(&request,&response);  //解析请求报文，设置对应的响应报文
        encodeResponse(&response,responseStr);   //responseStr中存放经过标准格式化后的响应报文
    }
    conn->connWrite(responseStr.c_str(),responseStr.size());    //发回响应报文
}

int HttpServer::findCRLF(const char* str)  //-2表示找到\r，-1表示\r和\r\n都没找到
{
    int res = 0;

    while(str[res+1] != '\0')
    {
        if(str[res] == '\r' && str[res+1] == '\n')
        {
            return res;
        }
        res++;
    }

    if(str[res] == '\r')  //报文到结尾
        return -2;
    return -1;
}

void HttpServer::parseRequestLine(httpRequest* request,const char* start,const char* end)
{
    switch(request->currentState_)
    {
        case REQUEST_STATUS:
        {
            sscanf(start,"%s %s %s\r\n",request->method_,request->url_,request->version_);
            request->currentState_ = REQUEST_HEADERS;
            break;
        }
        case REQUEST_HEADERS:
        {
            string str(start);
            char* pos = (char*)memmem(start,end-start,":",1);
            request->requestHeaders_[str.substr(0,pos-start)] = str.substr(pos-start+1,end-pos-1);
            break;
        }
        case REQUEST_BODY:
        {
            request->body_=start;
            request->currentState_ = REQUEST_DONE;
            break;
        }
        case REQUEST_DONE:
            break;
    }
}
int HttpServer::parseHttpRequest(const char* msg,httpRequest* request)
{
    int start = 0,end = 0;

    while((end = findCRLF(msg+start)) != -1)
    {
        if(!end)  //如果在开头，说明Headers已经解析结束，此时说明还有BODY段，如果没有BODY段，那么end为-2
        {
            request->currentState_ = REQUEST_BODY;
            start += 2;
        }
        else if(end == -2) //报文到结尾
        {
            request->currentState_ = REQUEST_DONE;
            start += 1;
        }
        parseRequestLine(request,msg+start,msg+start+end);
        start += end + 2;
    }

    if(request->currentState_!=REQUEST_DONE)  //如果在解析结束前就已经找不到CRLF，说明请求是无效的
    {
        return -1;
    }

    return 0;
}

void HttpServer::encodeResponse(httpResponse *response,string& responseStr)
{
    responseStr.clear();

    char writepos[4096];

    sprintf(writepos,"HTTP/1.1 %d %s\r\n",response->statusCode_,response->reason_);
    responseStr.append(writepos);
    if(response->Connect)
    {
        sprintf(writepos,"Content-Length: %ld\r\nConnection: Keep-Alive\r\n",response->body_.size());
    }
    else
    {
        sprintf(writepos,"Content-Length: %ld\r\nConnection: close\r\n",response->body_.size());
    }
    responseStr.append(writepos);
    for(auto &i:response->responseHeaders_)
    {
        sprintf(writepos,"%s: %s\r\n",i.first.c_str(),i.second.c_str());
        responseStr.append(writepos);
    }
    responseStr.append("\r\n");
    responseStr.append(response->body_);
    responseStr.append("\0");
}

void HttpServer::setRequestCallBack(const requestCb& cb)
{
    OnRequestCb = cb;
}