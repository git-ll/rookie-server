#include <iostream>
#include "base/AsyncLog.h"
#include <signal.h>
#include "net/HttpServer.h"
#include "net/HttpData.h"
#include "net/Util.h"
#include <mysql/mysql.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/stat.h>

using namespace rookie;
using namespace std;

static MYSQL db;

static string indexstr;
static string favstr;
static string jsstr;
static string cssstr;
static string regstr;
static string resetstr;

void OnRequest(httpRequest *request, httpResponse *response)   //request已经包含了Http的请求信息，response为响应信息
{
    if(request->requestGET())
    {
        if (request->isURL("/") || request->isURL("/index.html"))
        {
            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("text/html");
            if(!indexstr.empty())response->setBody(indexstr);
            else response->setBodyFromFile("resource/index.html");

        }
        else if (request->isURL("/favicon.ico"))
        {
            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("image/ico");
            if(!favstr.empty())response->setBody(favstr);
            else response->setBodyFromFile("resource/favicon.ico");
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
            if(!jsstr.empty())response->setBody(jsstr);
            else response->setBodyFromFile("resource/login.js");
        }
        else if (request->isURL("/login.css"))
        {
            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("text/css");
            if(!cssstr.empty())response->setBody(cssstr);
            else response->setBodyFromFile("resource/login.css");
        }
        else if (request->isURL("/register.html"))
        {
            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("text/html");
            if(!regstr.empty())response->setBody(regstr);
            else response->setBodyFromFile("resource/register.html");
        }
        else if (request->isURL("/resetpwd.html"))
        {
            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("text/html");
            if(!resetstr.empty())response->setBody(resetstr);
            else response->setBodyFromFile("resource/resetpwd.html");
        }
        else
        {
            const char* pos = nullptr;
            if ((pos = request->findInURL("/home.html")))
            {
                char acc[32];
                char query[128];
                char resstr[1024];
                MYSQL_RES* res = nullptr;
                MYSQL_ROW row;
                sscanf(pos,"/home.html&%s",acc);
                sprintf(query,"select nickname from users where username='%s'",acc);

                if(mysql_real_query(&db,query,strlen(query)))
                {
                    LOG_ERROR<<"Query failed ."<<mysql_error(&db);
                    return;
                }

                if(!(res = mysql_store_result(&db)))
                {
                    LOG_ERROR<<"Get result failed ."<<mysql_error(&db);
                    return;
                }

                if(!(row = mysql_fetch_row(res)))
                {
                    LOG_ERROR<<"fetch row failed ."<<mysql_error(&db);
                    return;
                }
                sprintf(resstr,"<!DOCTYPE html>    \
                                   <html lang=\"en\"> \
                                     <head>          \
                                      <meta charset=\"utf-8\">    \
                                      <title>ROOKIE</title>\
                                      <style>\
                                          h1 {color : red}\
                                          h2 {color : blue}\
                                      </style>\
                                      <h1 align=center>Welcome to ROOKIE Server!</h1>\
                                      <h2>个人信息</h2>\
                                      <text style=\"color:green\">账号：</text>%s<br></br> \
                                <text style=\"color:green\">昵称：</text>%s<br></br> \
                                    </head></html>",acc,row[0]);
                response->setStatus(OK);
                response->setStateReason("OK");
                response->setContentType("text/html");
                response->setBody(resstr);
                mysql_free_result(res);
            }
            else
            {
                response->setNotFound();
            }
            //response->setBodyFromFile("resource/home.html");
        }


    }
    else if(request->requestPOST())
    {
        if (request->isURL("/user/loginin"))  //登录
        {
            char username[32];
            char password[32];
            char query[128];
            MYSQL_RES* res = nullptr;
            MYSQL_ROW row;
            sscanf(UrlDecode(request->body()).c_str(),"ACC=%s PWD=%s",username,password);

            //数据库验证....
            sprintf(query,"select password,nickname from users where username='%s'",username);

            if(mysql_real_query(&db,query,strlen(query)))
            {
                LOG_ERROR<<"Query failed by ."<<mysql_error(&db);
            }

            if(!(res = mysql_store_result(&db)))
            {
                LOG_ERROR<<"Get result failed ."<<mysql_error(&db);
            }

            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("text/html");

            if(!res->row_count) //没有查找到
            {
                response->setBody("not exist");
            }
            else
            {
                if((row = mysql_fetch_row(res)))
                {
                    if(!strcmp(row[0],password))  //密码匹配
                    {
                        char resp[32];
                        sprintf(resp,"%s&%s",username,row[1]);
                        response->setBody(resp);
                    }
                    else  //密码不匹配
                    {
                        response->setBody("password error");
                    }
                }
                else
                {
                    LOG_ERROR<<"fetch row failed ."<<mysql_error(&db);
                    return;
                }
            }
            mysql_free_result(res);
        }
        else if(request->isURL("/user/register"))
        {
            char username[32] = {0};
            char nickname[32] = {0};
            char password[32] = {0};
            char identifier[11] = {0};
            char query[1024];
            MYSQL_RES* res = nullptr;
            sscanf(UrlDecode(request->body()).c_str(),"NICK=%s ACC=%s PWD=%s PIN=%s",nickname,username,password,identifier);

            //数据库插入....
            sprintf(query,"select id from users where username='%s'",username);

            if(mysql_real_query(&db,query,strlen(query)))
            {
                LOG_ERROR<<"Query failed ."<<mysql_error(&db);
                return;
            }

            if(!(res = mysql_store_result(&db)))
            {
                LOG_ERROR<<"Get result failed ."<<mysql_error(&db);
                return;
            }

            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("text/html");

            if(!res->row_count) //没有查找到
            {
                //当前username可以进行注册
                sprintf(query,"insert into users values(0,'%s','%s','%s','%s')",nickname,username,password,identifier);
                if(mysql_real_query(&db,query,strlen(query)))
                {
                    LOG_ERROR<<"Insert failed ."<<mysql_error(&db);
                    return;
                }
                response->setBody("ok");
            }
            else
            {
                response->setBody("exist");
            }
            mysql_free_result(res);
        }
        else if(request->isURL("/user/resetpwd"))
        {
            char username[32];
            char password[32];
            char identifier[11];
            char query[1024];
            MYSQL_RES* res = nullptr;
            MYSQL_ROW row;
            sscanf(UrlDecode(request->body()).c_str(),"ACC=%s PWD=%s PIN=%s",username,password,identifier);

            sprintf(query,"select identifier from users where username='%s'",username);

            if(mysql_real_query(&db,query,strlen(query)))
            {
                LOG_ERROR<<"Query failed ."<<mysql_error(&db);
                return;
            }

            if(!(res = mysql_store_result(&db)))
            {
                LOG_ERROR<<"Get result failed ."<<mysql_error(&db);
                return;
            }

            response->setStatus(OK);
            response->setStateReason("OK");
            response->setContentType("text/html");

            if(!res->row_count) //没有查找到
            {
                response->setBody("not exist");
            }
            else
            {
                if((row = mysql_fetch_row(res)))
                {
                    if(!strcmp(row[0],identifier))  //识别码匹配
                    {
                        sprintf(query,"update users set password='%s' where username='%s'",password,username);
                        if(mysql_real_query(&db,query,strlen(query)))
                        {
                            LOG_ERROR<<"update failed ."<<mysql_error(&db);
                            return;
                        }
                        response->setBody("ok");
                    }
                    else  //密码不匹配
                    {
                        response->setBody("pin error");
                    }
                }
                else
                {
                    LOG_ERROR<<"fetch row failed ."<<mysql_error(&db);
                    return;
                }
            }
            mysql_free_result(res);
        }
    }
}

string readStrFromFile(const char* path)
{
    int homefd = open(path,O_RDONLY);
    if(homefd < 0)
    {
        LOG_ERROR<<path<<" cannot found.";
        return "";
    }
    struct stat s = {};
    fstat(homefd,&s);
    char htmlstr[4096];
    if(read(homefd,htmlstr,s.st_size)<0)
    {
        LOG_ERROR<<"read failed from file "<<path;
        return "";
    }
    htmlstr[s.st_size]='\0';
    close(homefd);
    return string(htmlstr, s.st_size);
}

void strInit(string& indexstr,string& favstr,string& jsstr,string& cssstr,string& regstr,string& resetstr)
{
    indexstr = readStrFromFile("resource/index.html");
    favstr = readStrFromFile("resource/favicon.ico");
    jsstr = readStrFromFile("resource/login.js");
    cssstr = readStrFromFile("resource/login.css");
    regstr = readStrFromFile("resource/register.html");
    resetstr = readStrFromFile("resource/resetpwd.html");
}

int main(int argc, char* argv[])
{
    std::string addr,port,logpath;
    cout<<"Specify listening address or use \"0.0.0.0\" : ";
    getline(cin,addr);
    cout<<"Specify listening port or use 80 : ";
    getline(cin,port);
    cout<<"Specify listening logfile path or use \"./rookie_server\" : ";
    getline(cin,logpath);



    if(addr.empty())addr = "0.0.0.0";
    if(port.empty())port = "80";
    if(logpath.empty())logpath = "rookie_server";


    AsyncLog log(logpath,10000000);
    log.start();

    strInit(indexstr,favstr,jsstr,cssstr,regstr,resetstr);

    mysql_init(&db);    //初始化MYSQL变量
    if(mysql_real_connect(&db,"localhost","root","ll","userinfos",3306,NULL,0))  //连接数据库
    {
        LOG_INFO<<"Database connect success!";
    }
    else
    {
        LOG_FATAL<<"Database connect failed!";
    }

    if(mysql_set_character_set(&db,"utf8"))
    {
        LOG_FATAL<<"Database set_character failed!";
    }

    signal(SIGPIPE,SIG_IGN); //忽略SIGPIPE信号

    HttpServer server(addr.c_str(),atoi(port.c_str()));
    server.setRequestCallBack(OnRequest);
    server.start();

    /*TCPServer server("127.0.0.1",8888,4);
    server.start();*/
}


