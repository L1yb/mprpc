#include "logger.h"
#include <time.h>
#include <iostream>

mLogger& mLogger::GetInstance()
{
    static mLogger logger;
    return logger;
}
mLogger::mLogger()
{
    // 启动专门写日志的线程
    std::thread writeLogTask([&]()
    {
        for (;;)
        {
            // 获取当天的日期，然后取日志信息，写入相应的日志文件 a+
            time_t now = time(nullptr);
            tm* nowtm = localtime(&now);
            char file_name[128];
            sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday);

            FILE *pf = fopen(file_name, "a+");
            if (pf == nullptr)
            {
                std::cout << "logger file name:" << file_name << " open error." << std::endl;
                exit(EXIT_FAILURE);
            }
            std::string msg = m_lockqueue.Pop();
            char time_buf[128] = {0};
            sprintf(time_buf, "%d:%d:%d => [%s]", nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec, (m_loglevel == INFO ? "info" : "error"));
            msg.insert(0, time_buf);
            msg.append("\n");
            fputs(msg.c_str(), pf);
            fclose(pf);
        }
    });
    // 设置分离线程 守护线程
    writeLogTask.detach();
}
void mLogger::setLogLevel(LogLevel level)
{
    m_loglevel = level;
}
void mLogger::Log(std::string msg)
{
    m_lockqueue.Push(msg);
}