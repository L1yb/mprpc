#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <iostream>
#include <logger.h>
// 全局的watcher观察器 zkserver给zkclient的通知
void global_watcher(zhandle_t *zh, int type, int state, const char* path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)  // 回调的消息类型 和回话相关的消息类型
    {
        if (state == ZOO_CONNECTED_STATE)  // zkclient 和 zkclient连接成功
        {
            sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}


ZkClient::ZkClient() : m_zhandle(nullptr)
{
}
ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle);
    }
}
// 启动连接 zkserver
void ZkClient::Start()
{
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;

    /*
    zookeeper_mt: 多线程版本
    zookeeper的API客户端提供了三个线程：
    API调用线程 zookeeper_init（异步的） 所在线程
    网络I/O线程 pthread_create poll（没有使用epoll，客户端程序，需要高并发）
    watcher回调线程 pthread_create

    */
    // 这一步并不是初始化成功的结果，是创建一个句柄，session是否成功要等到接收到状态返回结果
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle)
    {
        // std::cout << "zookeeper_init error!" << std::endl;
        LOG_INFO("zookeeper_init error!");
        exit(EXIT_FAILURE);
    }
    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);

    sem_wait(&sem); // 程序阻塞到这里，  sem_post(sem) 得知是否成功
    std::cout << "zookeeper_init success." << std::endl;
    LOG_INFO("zookeeper_init success.");
    sem_destroy(&sem);
}
// 创建节点
void ZkClient::Create(const char* path, const char* data, int datalen, int state)
{
    char path_buf[128];
    int buffer_len = sizeof(path_buf);
    int flag;
    // 先判断path节点是否存在，不重复创建
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (flag == ZNONODE)
    {
        flag = zoo_create(m_zhandle, path, data, datalen, 
                        &ZOO_OPEN_ACL_UNSAFE, state, path_buf, buffer_len);
        if (flag == ZOK)
        {
            LOG_INFO("znode create success. path: %s", path);
        }
        else 
        {
            // std::cout << "flag:" << flag << std::endl;
            // std::cout << "znode create error. path: " << path << std::endl;
            LOG_ERR("flag:%d \nznode create error. path: %s", flag, path);
            exit(EXIT_FAILURE);
        }
                
    }
    else if (flag == ZOK)
    {
        LOG_INFO("znode already exists, skip creation. path: %s", path);
        
        // 如果是方法节点（包含临时节点标志），尝试更新数据
        if (state == ZOO_EPHEMERAL && data != nullptr)
        {
            flag = zoo_set(m_zhandle, path, data, datalen, -1);
            if (flag == ZOK)
            {
                LOG_INFO("znode data updated. path: %s", path);
            }
            else
            {
                LOG_ERR("flag:%d \nznode data update error. path: %s", flag, path);
            }
        }
    }
    else
    {
        LOG_ERR("flag:%d \nzoo_exists error. path: %s", flag, path);
        exit(EXIT_FAILURE);
    }
}
// 根据参数获取节点数据
std::string ZkClient::GetData(const char* path)
{
    char buffer[64] = {0};
    int buffer_len = sizeof(buffer);
    
    // 先检查节点是否存在
    int exists_flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (exists_flag != ZOK)
    {
        LOG_ERR("Node does not exist, path: %s, error code: %d", path, exists_flag);
        return "";
    }
    
    int flag = zoo_get(m_zhandle, path, 0, buffer, &buffer_len, nullptr);
    if (flag != ZOK)
    {
        LOG_ERR("get znode error. path: %s, error code: %d", path, flag);
        return "";
    }
    else
    {
        LOG_INFO("Got data from znode. path: %s, data: %s", path, buffer);
        return buffer;
    }
}