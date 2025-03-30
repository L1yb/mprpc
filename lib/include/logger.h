#pragma once
#include "lockqueue.h"
#include <string>

enum LogLevel
{
    INFO,
    ERROR
};
// mprpc框架的日志系统
class mLogger
{
public:
    static mLogger& GetInstance();

    void setLogLevel(LogLevel level);
    void Log(std::string msg);


private:
    int m_loglevel; // 记录日志级别
    LockQueue<std::string> m_lockqueue; // 日志缓冲队列

    mLogger();
    mLogger(const mLogger&) = delete;
    mLogger(mLogger&&) = delete;

};

// 定义宏
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        mLogger& logger = mLogger::GetInstance(); \
        logger.setLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    }while (0);

#define LOG_ERR(logmsgformat, ...) \
    do \
    {  \
        mLogger& logger = mLogger::GetInstance(); \
        logger.setLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    }while (0);
    
    