set(SRC_BASE
        LogStream.cpp
        LogOutput.cpp
        Thread.cpp
        ThreadLocalInfo.cpp
        LogFile.cpp
        CountDownLatch.cpp
        AsyncLog.cpp)

add_library(rookie_base ${SRC_BASE})   #将base下的源文件添加到rookie_base这个库中
target_link_libraries(rookie_base pthread)   #将pthread库链接到rookie_base库中

