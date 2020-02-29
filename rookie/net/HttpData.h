

#ifndef ROOKIE_HTTPDATA_H
#define ROOKIE_HTTPDATA_H

#include "HttpServer.h"
#include <vector>
#include <cstring>

namespace rookie
{
    enum requestState
    {
        REQUEST_STATUS,    //等待解析状态行
        REQUEST_HEADERS,   //等待解析headers
        REQUEST_BODY,      //等待解析请求body
        REQUEST_DONE       //解析完成
    };

    enum statusCode {
        Unknown,
        OK = 200,
        BadRequest = 400,
        NotFound = 404,
    };

    class httpRequest
    {
    public:
        httpRequest()
        :version_(""),
         method_(""),
         url_(""),
         body_(""),
         currentState_(REQUEST_STATUS)
        {
        }
        ~httpRequest() = default;

        bool requestGET();
        bool requestPOST();
        bool isURL(const char* url);
        const char* findInURL(const char* url);
        std::string& body();

    private:
        char version_[32];
        char method_[32];
        char url_[1024];
        std::string body_;
        requestState currentState_;
        std::map<std::string,std::string> requestHeaders_;

        friend HttpServer;
    };

    class httpResponse
    {
    public:
        httpResponse()
        :reason_(""),
         body_(""),
         statusCode_(statusCode::Unknown),
         Connect(true)
        {
        }

        void setStateReason(const char* str);

        void setBody(const std::string& str);

        void setBodyFromFile(const char* path);

        void makeDisConnect();

        void setContentType(const char* type);

        void addHeader(const char* key, const char* value);

        void setStatus(statusCode state);

        void setNotFound(const char* reason = "Cannot find resource.");

        const std::string& body(){ return body_;}

    private:
        std::string body_;
        statusCode statusCode_;
        char reason_[1024];
        std::map<std::string,std::string> responseHeaders_;
        bool Connect;

        friend HttpServer;
    };

}




#endif //ROOKIE_HTTPDATA_H
