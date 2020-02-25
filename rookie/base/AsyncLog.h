

#ifndef ROOKIE_ASYNCLOGOUTPUT_H
#define ROOKIE_ASYNCLOGOUTPUT_H

#include "noncopyable.h"
#include "Thread.h"
#include "LogStream.h"
#include "LogFile.h"
#include "Mutex.h"
#include "Condition.h"
#include "LogOutput.h"
#include "CountDownLatch.h"
#include <vector>
#include <memory>

namespace rookie
{
    using std::string;
    const size_t BUFSIZE = 4*1000*1000;  //前端缓冲区大小为4M
    class AsyncLog : noncopyable
    {

    public:
        AsyncLog(const string& fileBaseName,size_t rollFileSize,size_t flushInterval = 3);
        ~AsyncLog();

        void start();
        void stop();
        void append(const char* msg,size_t len);

    private:
        void logThreadFun();

        typedef LogStream<BUFSIZE>::BUFFER BUFFER;
        typedef std::unique_ptr<BUFFER> BUFPTR;
        typedef std::vector<BUFPTR>BUFPTRVEC;

        Thread logThread_;
        BUFPTR currentBuf_;    //当前使用的缓冲区
        BUFPTR reserveBuf_;    //预留的缓冲区
        BUFPTRVEC bufs_;   //前端缓冲区队列

        MutexLock mutex_;
        Condition cv_;
        bool running_;
        CountDownLatch latch_;

        //以下成员用于传递给LogFile类
        string baseName_;
        size_t rollFileSize;
        size_t flushInterval;

    };
}

#endif //ROOKIE_ASYNCLOGOUTPUT_H


