

#ifndef ROOKIE_LOGFILE_H
#define ROOKIE_LOGFILE_H

#include <string>
#include "Mutex.h"
#include <memory>


namespace rookie
{
    using std::string;

    class LogFile {

    public:
        explicit LogFile(const string& basename,size_t rollSize = 100*1000*1000/*100M*/ ,size_t flushInterval = 3,bool threadSafe = true);
        ~LogFile();

        void append(const char* msg,size_t len);
        //void appendnolock(const char* msg,size_t len);
        string getLogFileName(time_t& now);   //根据当前时间获取日志文件名称
        void rollFile();
        void openLogFile(string& logFileName);
        void closeLogFile();
        void flushLogFile();

        size_t getRollFileSize(){ return rollFileSize;}

    private:

        std::unique_ptr<MutexLock>mutex;   //锁，用来保证append的线程安全

        //日志滚动的两种情况：
        // 1、当前日志文件写入数据超过rollFileSize；
        // 2、每过一天滚动一次，日志滚动靠append函数来触发
        size_t rollFileSize;    //滚动阈值，达到该值就创建新的日志文件
        size_t fileSize;    //当前文件的大小
        FILE* fp;
        string logFileBaseName;   //日志的basename，后面滚动的新日志文件名都基于该名称

        time_t lastRollDay;     //上一次roll的天数
        time_t lastFlushTime;   //上一次flush的时间
        time_t lastRollTime;    //上一次roll的时间
        size_t flushInterval;   //文件的flush间隔,默认至少3秒flush一次，需要靠每次append来触发

        int count_;
        const int secondsPerDay = 60*60*24;
        static const int MaxRollFileSize = 100*1000*1000;   //默认初始日志大小为100M
    };
}




#endif //ROOKIE_LOGFILE_H
