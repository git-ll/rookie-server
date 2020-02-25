

#include <zconf.h>
#include "LogFile.h"
#include <assert.h>
#include <cstring>

using namespace rookie;

LogFile::LogFile(const string& basename, size_t rollSize, size_t flushInterval, bool threadSafe)
        :logFileBaseName(basename),
         rollFileSize(rollSize),
         flushInterval(flushInterval),
         mutex(threadSafe?new MutexLock: nullptr),
         fileSize(0),
         lastRollTime(0),
         lastFlushTime(0),
         lastRollDay(0),
         fp(nullptr),
         count_(0)
{
    time_t now = 0;
    string newFile = getLogFileName(now);
    openLogFile(newFile);
    lastRollDay = now/secondsPerDay;
    lastRollTime = now;
}

void LogFile::openLogFile(string& logFileName)
{
    assert(!fp);
    fp = ::fopen(logFileName.c_str(),"ae");
    assert(fp);
}

void LogFile::closeLogFile()
{
    assert(fp);
    //::fflush(fp);
    ::fclose(fp);//fclose会自动flush
    fp = nullptr;
}

void LogFile::flushLogFile()
{
    ::fflush(fp);
}

int rollcnt = 0;

LogFile::~LogFile()
{
    ::fclose(fp);
}

void LogFile::rollFile()
{
    time_t now = 0;
    string newFile = getLogFileName(now);
    if(now != lastRollTime)   //1s内多次滚动文件的结果还是同一文件
    {
        rollcnt++;
        closeLogFile();
        openLogFile(newFile);

        lastRollTime = now;
        fileSize = 0;
    }
}

string LogFile::getLogFileName(time_t& now)
{
    char timebuf[28];
    now = time(nullptr)+8*60*60;
    struct tm t = {};
    gmtime_r(&now,&t);

    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.rookie.log", &t);

    return logFileBaseName+timebuf;
}

void LogFile::append(const char* msg,size_t len)
{
    //MutexLockGuard m(*mutex);

    size_t nwritten = fwrite_unlocked(msg,1,len,fp);
    size_t remainbytes = len;
    while(nwritten < remainbytes)
    {
        size_t n = fwrite_unlocked(msg+nwritten,1,remainbytes,fp);    //避免频繁调用write，使用全缓冲fwrite，当且仅当flush和close时才会调用write
        if(!n)
        {
            fprintf(stderr,"LogFile::appendnolock failed!%s\n",strerror(ferror(fp)));
            break;
        }
        nwritten+=n;
        remainbytes-=n;
    }
    fileSize+=nwritten;

    if(fileSize>=rollFileSize)  //如果日志文件过大就滚动
    {
        rollFile();
    }
    else
    {
        count_++;
        if(count_>1024)
        {
            count_ = 0;
        }
        else return;
        time_t now(time(nullptr));
        time_t cur_day = now / secondsPerDay;
        if (cur_day - lastRollDay >= 1)     //如果是新的一天就roll
        {
            rollFile();
            lastRollDay = cur_day;
        } else if (now - lastFlushTime >= flushInterval)//超出flush间隔就roll
        {
            fflush(fp);
            lastFlushTime = now;
        }

    }
    //appendnolock(msg,len);
}

/*void LogFile::appendnolock(const char* msg, size_t len)
{
    size_t nwritten = fwrite_unlocked(msg,1,len,fp);
    size_t remainbytes = len;
    while(nwritten < remainbytes)
    {
        size_t n = fwrite_unlocked(msg+nwritten,1,remainbytes,fp);    //避免频繁调用write，使用全缓冲fwrite，当且仅当flush和close时才会调用write
        if(!n)
        {
            fprintf(stderr,"LogFile::appendnolock failed!%s\n",strerror(ferror(fp)));
            break;
        }
        nwritten+=n;
        remainbytes-=n;
    }
    fileSize+=nwritten;

    if(fileSize>=rollFileSize)  //如果日志文件过大就滚动
    {
        rollFile();
    }
    else
    {
        count_++;
        if(count_>1024)
        {
            count_ = 0;
        }
        else return;
        time_t now(time(nullptr));
        time_t cur_day = now / secondsPerDay;
        if (cur_day - lastRollDay >= 1)     //如果是新的一天就roll
        {
            rollFile();
            lastRollDay = cur_day;
        } else if (now - lastFlushTime >= flushInterval)//超出flush间隔就roll
        {
            flushLogFile();
            lastFlushTime = now;
        }

    }
}*/
