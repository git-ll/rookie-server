

#include "Thread.h"
#include <assert.h>
#include <memory>
#include "LogOutput.h"
#include "ThreadLocalInfo.h"
#include <sys/prctl.h>
#include <iostream>

namespace rookie
{
    int Thread::threadNum_ = 0;   //用来标识已经创建的线程数量

    Thread::Thread(ThreadFunc func,
                   const string& threadname)
            :func_(std::move(func)),
             threadName_(threadname),
             started_(false),
             joined_(false),
             tid_(0),
             pid_(0)
    {
        __sync_fetch_and_add(&threadNum_,1);
        if(threadName_.empty())
        {
            char buf[32];
            snprintf(buf, sizeof buf, "Thread%d", threadNum_);
            threadName_ = buf;
        }
    }

    Thread::~Thread()
    {
        if(started_&&!joined_)
            pthread_detach(pid_);
    }

    struct ThreadData    //由于真正的线程函数是threadStart，因此需要把我们需要执行的fun_作为参数传入threadStart中，ThreadData结构体就用来打包这些参数
    {
        ThreadData(pid_t* pid,string& s,Thread::ThreadFunc fun)
        :pid_(pid),
         name_(s),
         func_(std::move(fun)){}
        pid_t* pid_;     //主线程中的pid_是子线程的tid，因此传入pid_的指针，让子线程修改它，这样主线程就可以得到子线程的tid了
        string name_;
        Thread::ThreadFunc func_;
    };

    //由于Thread::start不能直接以func_作为线程函数(线程函数的形式是void*(void*)的，并且非静态成员函数不能作为线程函数)，
    // 因此就需要用另一个函数threadStart作为线程函数，来间接调用func_
    //如果把threadStart作为静态成员函数，那么它是无法调用非静态成员变量func_的，因此就把threadStart作为一个普通函数

    //另一种方法是直接设置func_的类型为void*(void*)类型，直接在start函数中调用func_，但是func_函数执行线程才是子线程，而func_函数的内容是未知的，
    //这样就无法设置子线程的name，也不能获取子线程的tid到Thread对象中了，因此只能使用threadStart。

    //threadStart的作用是：threadStart函数执行线程就是子线程，1、获取子线程的tid写入Thread对象中；2、根据Thread对象中的name去修改子线程的名字；3、执行func_
    void* threadStart(void *args)
    {

        auto data = static_cast<ThreadData*>(args);

        *(data->pid_) = ThreadLocalInfo::tid();
        ThreadLocalInfo::TL_threadName = data->name_.c_str();
        ::prctl(PR_SET_NAME, ThreadLocalInfo::TL_threadName);  //设置当前线程的名字

        try
        {
            data->func_();   //执行真正的线程函数
            ThreadLocalInfo::TL_threadName = "finished";
        }
        catch (const std::exception& ex)   //出现异常
        {
            ThreadLocalInfo::TL_threadName = "crashed";
            //fprintf(stderr, "exception caught in Thread %s\n", data->name_.c_str());
            //fprintf(stderr, "reason: %s\n", ex.what());
            //abort();
            LOG_FATAL<<"exception caught in Thread "<<data->name_.c_str()<<" reason : "<<ex.what();
        }
        catch (...)
        {
            ThreadLocalInfo::TL_threadName = "crashed";
            fprintf(stderr, "unknown exception caught in Thread %s\n", data->name_.c_str());
            throw; // rethrow
        }
    }

    void Thread::start()
    {
        assert(!started_);
        started_ = true;
        ThreadData* data = new ThreadData(&tid_,threadName_,func_);
        assert(data);
        //ThreadData data(&tid_,threadName_,func_);  //不要用栈对象，1、栈对象的地址传入线程函数作为参数，start结束后栈对象会析构，地址就是不安全的；2、同一类共用一套成员函数，因此多个Thread类中的data地址是相同的
        //std::unique_ptr<ThreadData>data(new ThreadData(&tid_,threadName_,func_));//这里不要使用智能指针,因为在子线程中还会再用到data，智能指针可能就将其释放了
        if(pthread_create(&pid_, nullptr,threadStart,data))
        {
            started_ = false;
            delete data;
            //未能创建成功,向日志中写入未能创建成功的线程的名字
            LOG_ERROR<<"cannot create thread:"<<threadName_;
        }
        else
            assert(pid_>0);
    }

    int Thread::join()   //join是否成功应该返回给用户，因为此时的线程已经创建，创建的结果如果失败直接写入日志返回即可,但是join失败就应该由用户来处理了
    {
        assert(started_);
        assert(!joined_);
        joined_ = true;
        return pthread_join(pid_, nullptr);
    }
}

