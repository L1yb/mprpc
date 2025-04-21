#include <string>
#include "rpcheader.pb.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include "mprpcapplication.h"
#include "zookeeperutil.h"
#include "logger.h"
/*
header_size(四个字节)   header_str   args_str
service_name method_name args_size
*/
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                              google::protobuf::RpcController* controller, 
                              const google::protobuf::Message* request,
                              google::protobuf::Message* response, 
                              google::protobuf::Closure* done)
{

    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    // 获取参数的序列化字符串长度 args_size
    std::string args_str;
    uint32_t args_size = 0;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();

    }
    else
    {
        controller->SetFailed("serialize request error.");
        return;
    }

    // 定义rpc请求的header
    mprpc::RpcHeader rpc_header;
    rpc_header.set_service_name(service_name);
    rpc_header.set_method_name(method_name);
    rpc_header.set_args_size(args_size);
    std::string rpc_header_str;
    uint32_t header_size = 0;
    if (rpc_header.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();

    }
    else
    {
        controller->SetFailed("serialize rpc_header error.");
        return;
    }

    // 组织待发送的rpc请求的字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;
    
    // std::cout << "================test==================" << std::endl;
    // std::cout << "header_size:" << header_size << std::endl;
    // std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    // std::cout << "service_name:" << service_name << std::endl;
    // std::cout << "method_name:" << method_name << std::endl;
    // std::cout << "args_str:" << args_str << std::endl;
    // std::cout << "==============test-end=================" << std::endl;


    // 使用tcp编程，完成发送
    // clientfd可以使用智能指针，自动维护
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        char err_text[512];
        snprintf(err_text, sizeof(err_text), "create socket error. errno:%d", errno);
        controller->SetFailed(err_text);
        return;
    }

    // 读取配置文件的rpcserver信息
    // std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    ZkClient zkCli;
    zkCli.Start();
    std::string method_path = "/" + service_name + "/" + method_name;
    LOG_INFO("Looking for service method at ZooKeeper path: %s", method_path.c_str());

    std::string host_data = zkCli.GetData(method_path.c_str());
    if (host_data == "")
    {
        std::string error_msg = method_path + " is not exist!";
        LOG_ERR("ZooKeeper node not found: %s", method_path.c_str());
        controller->SetFailed(error_msg);
        zkCli.Close(); // 显式关闭ZooKeeper连接
        return;
    }

    int idx = host_data.find(":");
    if (idx == -1)
    {
        std::string error_msg = method_path + " address is invalid!";
        LOG_ERR("Invalid address format in ZooKeeper data: %s", host_data.c_str());
        controller->SetFailed(error_msg);
        zkCli.Close(); // 显式关闭ZooKeeper连接
        return;
    }

    std::string ip = host_data.substr(0,idx);
    uint16_t port = atoi(host_data.substr(idx + 1, host_data.size() - idx).c_str());
    LOG_INFO("Found service at address: %s:%d", ip.c_str(), port);
    
    // 获取完服务信息后立即关闭ZooKeeper连接
    zkCli.Close();

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 发起连接
    if (-1 == connect(clientfd,(struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        close(clientfd);
        char err_text[512];
        snprintf(err_text, sizeof(err_text), "connect error. errno:%d", errno);
        controller->SetFailed(err_text);
        return;

    }

    // 发送请求
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        close(clientfd);
        char err_text[512];
        snprintf(err_text, sizeof(err_text), "send error. errno:%d", errno);
        controller->SetFailed(err_text);
        return;
    }

    // 接收rpc请求的响应值
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {   
        close(clientfd);
        char err_text[512];
        snprintf(err_text, sizeof(err_text), "receive error. errno:%d", errno);
        controller->SetFailed(err_text);
        return;
    }
    // std::string response_str(recv_buf, 0, recv_size); // bug， 读取到/0就停止了
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        close(clientfd);
        char err_text[512];
        snprintf(err_text, sizeof(err_text), "parse error. response_str:%s", recv_buf);
        controller->SetFailed(err_text);
        return;
    }
    close(clientfd);
    
    LOG_INFO("RPC call completed successfully");
}