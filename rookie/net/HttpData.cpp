

#include "HttpData.h"
#include <fcntl.h>
#include <sys/stat.h>
#include "../base/LogOutput.h"

using namespace rookie;

//httpRequest

bool httpRequest::requestGET()
{
    return !strcmp(method_,"GET");
}

bool httpRequest::requestPOST()
{
    return !strcmp(method_,"POST");
}

bool httpRequest::isURL(const char* url)
{
    return !strcmp(url_,url);
}

const char* httpRequest::findInURL(const char* url)
{
    return strstr(url_,url);
}

std::string& httpRequest::body()
{
    return body_;
}


//httpResponse

void httpResponse::setStateReason(const char* str)
{
    strncpy(reason_,str,strlen(str));
}

void httpResponse::setBody(const std::string& str)
{
    body_ = str;
}

void httpResponse::setBodyFromFile(const char* path)
{
    int homefd = open(path,O_RDONLY);
    if(homefd < 0)
    {
        LOG_ERROR<<path<<" cannot found.";
        setNotFound();
        return;
    }
    struct stat s = {};
    fstat(homefd,&s);
    char* htmlstr = new char[s.st_size];
    if(read(homefd,htmlstr,s.st_size)<0)
    {
        LOG_ERROR<<"read failed from file "<<path;
        setNotFound();
        return;
    }
    htmlstr[s.st_size]='\0';
    setBody(string(htmlstr, s.st_size));

    delete[]htmlstr;
    close(homefd);
}

void httpResponse::makeDisConnect()
{
    Connect = false;
}

void httpResponse::setContentType(const char* type)
{
    addHeader("Content-Type",type);
}

void httpResponse::addHeader(const char* key, const char* value)
{
    responseHeaders_[key] = value;
}

void httpResponse::setStatus(statusCode state)
{
    statusCode_ = state;
}

void httpResponse::setNotFound(const char* reason)
{
    statusCode_ = NotFound;
    setStateReason(reason);
    makeDisConnect();
    body_ = "404 Not Found";
}

