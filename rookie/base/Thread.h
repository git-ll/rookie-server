

#ifndef ROOKIE_THREAD_H
#define ROOKIE_THREAD_H

#include "noncopyable.h"
#include <functional>
#include <linux/types.h>

namespace rookie
{
    using std::string;
    class Thread
    {
    public:
        typedef std::function<void ()> ThreadFunc;//限制了线程函数的返回值类型和参数类型为void(void)

        //explicit Thread(ThreadFunc,std::string&& threadname = "");     //右值构造
        explicit Thread(ThreadFunc,const string& threadname = "");
        ~Thread();

        void start();
        int join();

        static int count(){ return threadNum_ ;}
        const string& name(){ return threadName_;}
        bool started(){ return started_;}


    private:
        pthread_t pid_;
        /*以下信息是子线程(线程函数所在线程)的信息*/
        pid_t tid_;
        string threadName_;     //线程函数所在线程名字
        bool started_;
        bool joined_;
        ThreadFunc func_;
        static int threadNum_;
    };
}



#endif //ROOKIE_THREAD_H
