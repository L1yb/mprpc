#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
int main(int argc, char** argv)
{
    // 框架初始化
    MprpcApplication::Init(argc, argv);    
    // 演示调用rpc方法的login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    
    // rpc方法请求参数
    fixbug::LoginRequest request;
    request.set_name("ali");
    request.set_pwd("123123");

    // rpc方法的回应
    fixbug::LoginResponse response;

    // 代理对象发起rpc方法的调用 同步rpc调用过程 rpcchannel->callmethod
    stub.Login(nullptr, &request, &response, nullptr);

    // 一次调用完成， 读调用结果
    if (response.rscode().err_code() == 0)
    {
        std::cout << "rpc login response success: " << response.success() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error: " << response.rscode().err_msg() << std::endl;

    }
    
    // 演示调用rpc方法的register
    fixbug::RegisterRequest register_req;
    register_req.set_id(2000);
    register_req.set_name("mprpc");
    register_req.set_pwd("123123");
    fixbug::RegisterResponse register_rsp;
    stub.Register(nullptr, &register_req, &register_rsp, nullptr);
    if (register_rsp.rscode().err_code() == 0)
    {
        std::cout << "rpc register register_rsp success: " << register_rsp.success() << std::endl;
    }
    else
    {
        std::cout << "rpc register register_rsp error: " << register_rsp.rscode().err_msg() << std::endl;

    }


    return 0;
}