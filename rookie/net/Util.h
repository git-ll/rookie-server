

#ifndef ROOKIE_TIMEUTIL_H
#define ROOKIE_TIMEUTIL_H

#include "Channel.h"
#include <bits/time.h>
#include <fcntl.h>
// TimeUtil

timeval timeadd(timeval p,timeval q);

timeval timeadd(timeval p,int millisecond);

timeval timesub(timeval p,timeval q);

bool greater(timeval p,timeval q);

//SocketUtil

int setSocketNonBlocking(int fd);

//FileUtil

int readFile(const char* filepath,char* buf);//读取指定路径的整个文件到buf中，返回文件读取长度

//URI Code

unsigned char ToHex(unsigned char x);

unsigned char FromHex(unsigned char x);

std::string UrlEncode(const std::string& str);     //将url进行编码

std::string UrlDecode(const std::string& str);     //将url进行解码


#endif //ROOKIE_TIMEUTIL_H
