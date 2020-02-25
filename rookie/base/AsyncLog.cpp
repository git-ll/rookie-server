#include "AsyncLog.h"
#include <assert.h>
#include <sys/time.h>

using namespace rookie;

AsyncLog* p = nullptr;

void AsyncLogOutFun(const char* msg,size_t len)
{
    p->append(msg,len);
}

AsyncLog::AsyncLog(const string& fileBaseName, size_t rollFileSize, size_t flushInterval)
         :logThread_(std::bind(&AsyncLog::logThreadFun,this),"AsyncLogThread"),
          baseName_(fileBaseName),
          rollFileSize(rollFileSize),
          flushInterval(flushInterval),
          mutex_(),
          cv_(mutex_),
          currentBuf_(new BUFFER),
          reserveBuf_(new BUFFER),
          running_(false),
          latch_(1)
{
    currentBuf_->makezero();
    reserveBuf_->makezero();
    bufs_.reserve(16);
    p = this;
    LogOutput::setOutputFun(AsyncLogOutFun);
}

AsyncLog::~AsyncLog()
{
    if(running_)
        stop();
}

void AsyncLog::start()
{
    running_ = true;
    logThread_.start();
    latch_.wait();   //等到线程启动再返回
}

void AsyncLog::stop()
{
    running_ = false;
    cv_.notify();
    logThread_.join();
}

void AsyncLog::append(const char *msg, size_t len)  //将msg添加到前端缓冲区currentBuf中，当currentBuf满了就将其放到bufs_中，并且唤醒后台日志线程进行写出
{
    MutexLockGuard m(mutex_);
    if(currentBuf_->remain() > len)  //如果currentBuf还放得下就直接放在currentBuf中
    {
        currentBuf_->appendnocheck(msg,len);
    }
    else //如果currentBuf放不下了，就看看reserveBuf是否还有空间
    {
        bufs_.push_back(std::move(currentBuf_));
        if(reserveBuf_)   //如果reserveBuf_可用，就直接用它
        {
            currentBuf_ = std::move(reserveBuf_);
        }
        else  //reserveBuf_不可用，就重新分配
        {//这种情况很少发生，每条日志消息的最大长度为4k，而每个缓冲区的最大长度为4M，即使连续多个线程进行append，也很难让4M的缓冲区爆满
            currentBuf_ = std::make_unique<BUFFER>();
           // reserveBuf_ = std::make_unique<BUFFER>();
           // 把reserveBuf的重新分配放到日志线程中进行，因为append的调用频率会很高，只需要保证currentBuf空闲即可（reserveBuf的分配没有必要），在日志线程中检查reserveBuf即可
        }
        currentBuf_->append(msg,len);   //此时currentBuf_中有新的空间，因此重新append
        cv_.notify();
    }
}

void AsyncLog::logThreadFun()   //日志后台写出线程
{
    assert(running_);
    latch_.countDown();
    LogFile outfile(baseName_,rollFileSize,flushInterval);     //每一个日志线程所写的文件不同
    BUFPTR newBuf1(new BUFFER);  //如果currentbuf未满，但是被超时唤醒，那么newBuf1用来填补currentbuf；
    BUFPTR newBuf2(new BUFFER);  //如果currentbuf已满，那么currentbuf在append中就会被reservebuf填补，因此newBuf1用来填补reservebuf。
    newBuf1->makezero();
    newBuf2->makezero();
    BUFPTRVEC bufToWrite;
    bufToWrite.reserve(16);
    while(running_)
    {
        assert(newBuf1);
        assert(newBuf2);

         {
            //用于检查此时有哪些buffer需要被写到后端，使用bufToWrite可以缩短加锁范围
            MutexLockGuard m(mutex_);

            if(bufs_.empty())
            {
                cv_.waitForSeconds(flushInterval);   //每次苏醒都必定会flush，因此直接用flushInterval替代睡眠时间
            }
            bufs_.push_back(std::move(currentBuf_));
            currentBuf_ = std::move(newBuf1);

            if(!reserveBuf_)
            {
                reserveBuf_ = std::move(newBuf2);
            }

            bufs_.swap(bufToWrite);
            //此后bufs_的内容全部移入bufToWrite中，bufs_重新变成空的，解锁后其它线程可以继续向bufs_中添加日志而不会影响后面写出到文件
        }
        for (auto &buf : bufToWrite)
        {
            outfile.append(buf->data(), buf->length());
        }

        //bufToWrite已经写出到外部文件，那么此时bufToWrite的元素可以再重新利用
        //printf("size : %ld\n",bufToWrite.size());
        if(!newBuf1)
        {
            newBuf1 = std::move(bufToWrite.back());  //从bufToWrite中回收资源
            newBuf1->reset();
        }

        if (!newBuf2)//如果newBuffer2为空
        {
            newBuf2 = std::move(bufToWrite.front());
            newBuf2->reset();
        }

        bufToWrite.clear();
        outfile.flushLogFile();
    }
}