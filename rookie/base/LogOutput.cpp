

#include <stdio.h>
#include <assert.h>
#include "LogOutput.h"
#include "ThreadLocalInfo.h"
#include <time.h>
#include <sys/time.h>
#include "LogStream.h"

using namespace rookie;

void defaultOutputFun(const char * msg,size_t len)
{
    size_t n = fwrite(msg,1,len,stdout);

    assert(n>0);
}

void defaultFlushFun()
{
    ::fflush(stdout);    //把stdout流中的数据立刻冲刷到终端
}

LogOutput::OutputFunc logOutputFun = defaultOutputFun;
LogOutput::FlushFunc logFlushFun = defaultFlushFun;
LogLevel LogOutput::Lv = LogLevel::INFO;

const char* LogLevelstr[LogLevel::LogLevelNum] =
        {
            " DEBUG ",   //调试信息，用来辅助理解程序执行过程，在实际运行时不需要输出，比如某个变量的值
            " INFO  ",  //普通消息，通知用户一个操作或状态发生了变化，比如“xxx初始化成功”、“成功连接服务器”、“客户端断开”之类的
            " WARN  ",  //警告消息，可能存在潜在的错误
            " ERROR ",   //有错误事件发生，但是不会影响系统继续运行
            " FATAL ",   //有错误事件发生，会导致系统停止运行
        };

void LogOutput::setOutputFun(LogOutput::OutputFunc out)
{
    logOutputFun = out?out:defaultOutputFun;
}

void LogOutput::setFlushFun(LogOutput::FlushFunc flush)
{
    logFlushFun = flush?flush:defaultFlushFun;
}

void LogOutput::setLogLevel(LogLevel lv)
{
    Lv = lv;
}

LogLevel LogOutput::getLogLevel()
{
    return Lv;
}

LogOutput::LogOutput(const char* srcFileName,size_t line,LogLevel logLevel,int syserror):err(syserror),cur_Level(logLevel)
{

    gettimenow(); //获取当前时间字符串到timestr_中

    srcFileName = strrchr(srcFileName, '/');
    srcFileName++;
    stream_<<timestr_<<ThreadLocalInfo::tid()<<LogLevelstr[logLevel]<<srcFileName<<':'<<line<<' ';
}

LogOutput::~LogOutput()
{
    if(err!=0)
        stream_ << ':'<<strerror(err);
    stream_<<'\n';
    logOutputFun(stream_.data(),stream_.length());
    if(cur_Level == LogLevel::FATAL)
    {
        logFlushFun(); //默认的输出流是stdout，为行缓冲，遇到换行符或者满行就会输出，虽然前面已经加入了换行符，但是如果stream_中本身就有一个换行符，那么就会剩下一个换行符在缓冲中，因此调用fflush来清空缓冲区
        abort();
    }
}

void LogOutput::gettimenow()
{
    struct timeval tv = {};
    gettimeofday(&tv,nullptr);  //获取微秒时间
    tv.tv_sec+=8*60*60;

    struct tm t = {};
    gmtime_r(&tv.tv_sec,&t);

    snprintf(timestr_, sizeof timestr_,"%04d%02d%02d %02d:%02d:%02d:%06ld ", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,tv.tv_usec);
}

