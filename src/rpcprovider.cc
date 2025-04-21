#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"
/*
生成一个表，记录哪个服务对象调用什么方法
service_name => service* 服务对象
                    methodCnt =》 方法描述

json: 文本存储 键值对存储除信息外还要存储"键"的信息
protobuf： 二进制存储（效率更高）， 紧密存储只携带数据信息 还提供了service rpc的描述
*/
// 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{

    ServiceInfo service_info;

    //获取对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    //获取服务名称
    std::string service_name = pserviceDesc->name();
    // std::cout << "service name: " << service_name << std::endl;
    LOG_INFO("service name: %s", service_name.c_str());
    //获取服务对象Service的方法的数量
    int methodCnt = pserviceDesc->method_count();
    //获取服务对象指定下标的方法的描述，抽象的描述
    for (int i = 0; i < methodCnt; i++)
    {
        const google::protobuf::MethodDescriptor* pmethodDesc= pserviceDesc->method(i);        
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
        // std::cout << "method name: " << method_name << std::endl; 
        LOG_INFO("method name: %s", method_name.c_str());

    }
    service_info.service = service;
    m_serviceMap.insert({service_name, service_info});


}


// 提供rpc远程网络调用服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);
    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventloop, address, "RpcProvider");
    // 绑定连接回调和消息读写回调 分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, 
                                            std::placeholders::_2, std::placeholders::_3));
    // 设置muduo库线程数
    server.setThreadNum(4);

    // 把rpc节点上要发布的服务全部注册到zk上， 让rpc和client可以在zk上发现服务
    ZkClient zkCli;
    zkCli.Start();
    
    LOG_INFO("Starting to register services to ZooKeeper, total services: %lu", m_serviceMap.size());
    
    for (auto &sp : m_serviceMap)
    {
        std::string service_path = '/' + sp.first;
        LOG_INFO("Registering service: %s", sp.first.c_str());
        zkCli.Create(service_path.c_str(), nullptr, 0);
        
        LOG_INFO("Service %s has %lu methods to register", sp.first.c_str(), sp.second.m_methodMap.size());
        
        for (auto &mp : sp.second.m_methodMap)
        {
            std::string method_path = service_path + '/' + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            
            LOG_INFO("Registering method: %s with address: %s", mp.first.c_str(), method_path_data);
            
            // ZOO_EPHEMERAL表示是临时性节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    LOG_INFO("All services registered to ZooKeeper successfully");
    LOG_INFO("RpcProvider start service at ip:%s port:%d", ip.c_str(), port);
    
    // 启动网络服务
    server.start();
    m_eventloop.loop();
    
}
// 新的socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if (!conn->connected())
    {
        conn->shutdown();
    }
}

/*
已建立连接用户的读写时间回调
如果远端发送了一个rpc服务的调用请求，那么 OnMessage方法就会响应

在框架内部，RpcProvider和RpcComsumer协商好之间通信使用的protobuf数据类型
应包含 service_name method_name args_size
定义proto的message类型，进行数据头的系列化和反序列化
UserServiceLoginzhangsan123456
header_size(四个字节)   header_str   args_str
std::string insert和copy方法
*/
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buffer,
                            muduo::Timestamp)
{
    // 网络上接受远程rpc调用请求的字符流 
    std::string recv_buf = buffer->retrieveAllAsString();

    // 从字符流中读取四个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);

    // 根据 header_size 读取原始数据流， 反序列化数据， 得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    
    mprpc::RpcHeader rpcheader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcheader.ParseFromString(rpc_header_str))
    {
        service_name = rpcheader.service_name();
        method_name = rpcheader.method_name();
        args_size = rpcheader.args_size();      
    }
    else
    {
        // std::cout << "rpc_header_str:" << rpc_header_str << " parse error" << std::endl;
        LOG_INFO("rpc_header_str: %s parse error", rpc_header_str.c_str())
        return;
    }
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // std::cout << "================test==================" << std::endl;
    // std::cout << "header_size: " << header_size << std::endl;
    // std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    // std::cout << "service_name: " << service_name << std::endl;
    // std::cout << "method_name: " << method_name << std::endl;
    // std::cout << "args_str: " << args_str << std::endl;
    // std::cout << "==============test-end=================" << std::endl;

    // 获取service对象和 method 对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        // std::cout << service_name << " is not exist." << std::endl;
        LOG_INFO("%s is not exist.", service_name.c_str());
        return;
    }

    google::protobuf::Service *service = it->second.service;

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        // std::cout << service_name << ":" << method_name <<" is not exist." << std::endl;
        LOG_INFO("%s:%s is not exist.", service_name.c_str(), method_name.c_str());
        return;
    }

    const google::protobuf::MethodDescriptor* method = mit->second;

    // 生成rpc方法调用的请求request和响应 response
    google::protobuf::Message* request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        // std::cout << "request prase error. content:" << args_str << std::endl;
        LOG_INFO("request prase error. content:%s", args_str.c_str());
        return;
    }
    google::protobuf::Message* response = service->GetResponsePrototype(method).New();

    // 给下面方法调用 绑定一个closure回调
    google::protobuf::Closure* done = google::protobuf::NewCallback<RpcProvider, const muduo::net::TcpConnectionPtr&, google::protobuf::Message*>(this, &RpcProvider::SendRpcResponse, conn, response);

    // 在框架上根据远程rpc请求，调用当前rpc节点上发布的方法
    service->CallMethod(method, nullptr, request, response, done);

}


// Closure回调操作，用于序列化响应和网络发送
void  RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))
    {
        conn->send(response_str);
    }
    else
    {
        // std::cout << "Serialize response_str error." << std::endl;
        LOG_INFO("Serialize response_str error.");
    }
    // 模拟http 短连接服务，由rpcprovider主动断开连接
    conn->shutdown();
}

