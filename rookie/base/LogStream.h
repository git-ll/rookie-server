

#ifndef ROOKIE_LOGSTREAM_H
#define ROOKIE_LOGSTREAM_H

#include <string>
#include <string.h>
#include "noncopyable.h"
#include <algorithm>


extern const char digits[];
extern const char* zero;
extern const char digitsHex[];

template <typename T>
size_t convert(char buf[], T value)  //把数字value转换为字符串，存放到buf中
{
    T i = value;
    char* p = buf;

    do
    {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);//把数字从低位到高位转换为字符串

    if (value < 0)//如果是负数在末尾添加负号
    {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);//反转

    return p - buf; //返回字符串的长度
}

namespace rookie
{
    using std::string;

    template<size_t SIZE>
    class BaseBuffer : noncopyable
    {
    public:
        BaseBuffer():pos_(buffer_){ makezero();};
        ~BaseBuffer() = default;


        const char * data() const{ return buffer_ ;} //缓冲区数据
        size_t length() const { return static_cast<size_t >(pos_ - buffer_) ; } //当前缓冲区数据长度
        char * current() const { return pos_ ;}

        size_t remain() const { return buffer_ + SIZE - pos_ ; }   //缓冲区剩余空间大小

        size_t capacity() const { return SIZE;}    //缓冲区容量
        void reset() { makezero(); pos_ = buffer_ ; }    //相当于清空缓冲区

        string toString() const { return std::string(buffer_,length());}

        void append(const char* buf,size_t len)  //如果len大于buf的实际长度，那么中间就会被‘\0’占据
        {
            if(remain() > len)
            {
                memcpy(pos_,buf,len);
                pos_ += len;
            }
        }
        void appendnocheck(const char*buf,size_t len)
        {
            memcpy(pos_,buf,len);
            pos_+=len;
        }

        void makezero() { memset(buffer_,0,SIZE) ;}   //清零

    protected:
        void add(size_t n){ pos_ += n ; }
    private:
        char* pos_;   //当前空闲缓冲区位置
        char buffer_[SIZE];    //缓冲区内容
    };
    template <size_t BUFSIZE>
    class LogStream
            : public BaseBuffer<BUFSIZE>     //直接继承BaseBuffer
    {
    public:
        typedef BaseBuffer<BUFSIZE>BUFFER;
        typedef LogStream<BUFSIZE>self;

        self& operator<<(bool v)
        {
            this->append(v?"1":"0",1);
            return *this;
        }
        self& operator<<(char v)
        {
            this->append(&v, 1);
            return *this;
        }
        self& operator<<(const char * buf)
        {
            if(buf)
                this->append(buf,strlen(buf));
            else
                this->append("(null)",6);
            return *this;
        }
        self& operator<<(string& str)
        {
            this->append(str.c_str(),str.size());
            return *this;
        }
        self& operator<<(const string& str)
        {
            this->append(str.c_str(),str.size());
            return *this;
        }
        self& operator<<(string&& str)
        {
            this->append(str.c_str(),str.size());
            return *this;
        }
        self& operator<<(const BUFFER& buf)
        {
            string s(std::move(buf.toString()));
            *this<<s;
            return *this;
        }
        self& operator<<(short v)
        {
            appendInteger(v);
            return *this;
        }
        self& operator<<(unsigned short v)
        {
            appendInteger(v);
            return *this;
        }
        self& operator<<(int v)
        {
            appendInteger(v);
            return *this;
        }
        self& operator<<(unsigned int v)
        {
            appendInteger(v);
            return *this;
        }
        self& operator<<(long v)
        {
            appendInteger(v);
            return *this;
        }
        self& operator<<(unsigned long v)
        {
            appendInteger(v);
            return *this;
        }
        self& operator<<(long long v)
        {
            appendInteger(v);
            return *this;
        }
        self& operator<<(unsigned long long v)
        {
            appendInteger(v);
            return *this;
        }
        self& operator<<(float f)
        {
            appendFloat(f);
            return *this;
        }
        self& operator<<(double f)
        {
            appendFloat(f);
            return *this;
        }

    private:

        template <typename T>
        void appendInteger(T v) //将整形数添加到stream中
        {
            if(this->remain() >= MaxNumLength)
            {
                size_t len = convert(this->current(),v);
                this->add(len);
            }
        }

        template  <typename T>
        void appendFloat(T f) //将浮点数添加到stream中
        {
            if(this->remain() >= MaxNumLength)
            {
                int len = snprintf(this->current(),MaxNumLength,"%.12g",f);  //保留12位有效数字的科学计数法
                this->add(len);
            }
        }

        static const int MaxNumLength = 32;   //数字对应的字符串最大长度
    };
}

#endif
