#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    // 启动连接 zkserver
    void Start();
    // 创建节点
    void Create(const char* path, const char* data, int datalen, int state = 0);
    // 根据参数获取节点数据
    std::string GetData(const char* path);
private:
    // 客户端句柄
    zhandle_t *m_zhandle;

};