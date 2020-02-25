set(SRC_NET
        EventLoop.cpp
        Socket.cpp
        Channel.cpp
        Epoll.cpp
        Timer.cpp
        Util.cpp
        EventLoopThread.cpp
        EventLoopThreadPool.cpp
        TimerHeap.cpp
        TCPServer.cpp
        Connection.cpp
        HttpServer.cpp HttpData.h HttpData.cpp)

include_directories("../base/")

add_library(rookie_net ${SRC_NET})   #将net下的源文件添加到rookie_base这个库中
target_link_libraries(rookie_net rookie_base)   #将pthread库链接到rookie_base库中

