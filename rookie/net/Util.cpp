

#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include "Util.h"

timeval timeadd(timeval p,timeval q)
{
    p.tv_usec += q.tv_usec;
    p.tv_sec += q.tv_sec;
    if(p.tv_usec>999999)
    {
        p.tv_sec += p.tv_usec/1000000;
        p.tv_usec %= 1000000;
    }
    return p;
}

timeval timeadd(timeval p,int millisecond)
{
    p.tv_usec+=millisecond*1000;
    if(p.tv_usec>999999)
    {
        p.tv_sec+=p.tv_usec/1000000;
        p.tv_usec %= 1000000;
    }
    return p;
}

timeval timesub(timeval p,timeval q)
{
    p.tv_usec -= q.tv_usec;
    p.tv_sec -= q.tv_sec;
    if(p.tv_usec<0)
    {
        p.tv_usec+=1000000;
        p.tv_sec--;
    }
    return p;
}

bool greater(timeval p,timeval q)
{
    if(p.tv_sec>q.tv_sec)
    {
        return true;
    }
    else if(p.tv_sec<q.tv_sec)
    {
        return false;
    }

    return p.tv_usec>=q.tv_usec;
}

int setSocketNonBlocking(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    if(flag == -1)
        return -1;

    flag |= O_NONBLOCK;

    if(fcntl(fd, F_SETFL, flag) == -1)
        return -1;
    return 0;
}

unsigned char ToHex(unsigned char x)
{
    return  x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x)
{
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else assert(0);
    return y;
}

std::string UrlEncode(const std::string& str)
{
    std::string strTemp;
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ')
            strTemp += "+";
        else
        {
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] % 16);
        }
    }
    return strTemp;
}

std::string UrlDecode(const std::string& str)
{
    std::string strTemp;
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (str[i] == '+') strTemp += ' ';
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strTemp += high*16 + low;
        }
        else if(str[i] == '&')
            strTemp+= ' ';      //方便读取
        else strTemp += str[i];
    }
    return strTemp;
}