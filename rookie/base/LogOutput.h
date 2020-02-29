

#ifndef ROOKIE_LOGOUTPUT_H
#define ROOKIE_LOGOUTPUT_H

#include <stddef.h>
#include "noncopyable.h"
#include "LogStream.h"
#include <functional>

namespace rookie
{
    const size_t MSGSIZE = 4000;   //每一条日志消息的大小为4K

    typedef enum
    {
        DEBUG,  //调试信息
        INFO,   //提示信息
        WARNING,   //警告
        ERROR,  //错误
        FATAL,  //严重错误
        LogLevelNum  //日志级别数量
    }LogLevel;

    class LogOutput : noncopyable
    {
    public:
        typedef void (*OutputFunc)(const char* msg, size_t len);
        typedef void (*FlushFunc)();

        static void setOutputFun(OutputFunc);
        static void setFlushFun(FlushFunc);
        static void setLogLevel(LogLevel);
        static LogLevel getLogLevel();


        LogOutput(const char* srcFileName,size_t line,LogLevel logLevel,int syserror=0);

        ~LogOutput();

        LogStream<MSGSIZE>& stream() { return stream_;}

    private:
        void gettimenow();
        LogStream<MSGSIZE> stream_;  //缓冲区
        char timestr_[30];  //时间字符串
        static LogLevel Lv;  //全局日志等级，高于或等于该等级才会显示
        LogLevel cur_Level; //当前日志消息等级
        int err;
    };

#define LOG_DEBUG  if(LogOutput::getLogLevel() == rookie::LogLevel::DEBUG)   \
                    rookie::LogOutput(__FILE__,__LINE__, rookie::LogLevel::DEBUG).stream()

#define LOG_INFO if(LogOutput::getLogLevel() <= rookie::LogLevel::INFO)   \
                    rookie::LogOutput(__FILE__,__LINE__, rookie::LogLevel::INFO).stream()

#define LOG_WARN rookie::LogOutput(__FILE__, __LINE__, rookie::LogLevel::WARNING).stream()
#define LOG_ERROR rookie::LogOutput(__FILE__,__LINE__, rookie::LogLevel::ERROR,errno).stream()
#define LOG_FATAL rookie::LogOutput(__FILE__,__LINE__, rookie::LogLevel::FATAL,errno).stream()

//errno是thread-local的，因此是线程安全的

}



#endif //ROOKIE_LOGOUTPUT_H
