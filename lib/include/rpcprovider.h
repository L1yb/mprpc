#pragma once
#include "google/protobuf/service.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpConnection.h"
#include <string>
#include "functional"
#include "google/protobuf/descriptor.h"
#include <unordered_map>
// 框架提供的专门负责 发布rpc服务 的 网络对象
class RpcProvider
{
public:
    // 框架提供的外部接口，可以发布rpc方法
    void NotifyService(google::protobuf::Service *service);

    // 启动rpc发布节点，阻塞线程，等待rpc调用请求
    void Run();
private:
    std::unique_ptr<muduo::net::TcpServer> m_tcpserver;
    muduo::net::EventLoop m_eventloop; 

    // 保存一个service服务信息，包括方法
    struct ServiceInfo
    {
        google::protobuf::Service *service;
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap;
    };
    // 保存所有的服务信息
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;


    // 新的socket连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);
    // 读写信息的回调
    void OnMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*,
                    muduo::Timestamp);
    // Closure回调操作，用于序列化响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);
};