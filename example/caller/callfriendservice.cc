#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"


int main(int argc, char** argv)
{
    // 框架初始化
    MprpcApplication::Init(argc, argv);    
    // 演示调用rpc方法的login
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());
    
    // rpc方法请求参数
    fixbug::GetFriendListRequest get_fri_request;
    get_fri_request.set_id(2000);

    // rpc方法的回应
    fixbug::GetFriendListResponse get_fri_response;

    // 代理对象发起rpc方法的调用 同步rpc调用过程 rpcchannel->callmethod
    MprpcController controller;
    stub.GetFriendList(&controller, &get_fri_request, &get_fri_response, nullptr);

    // 一次调用完成， 读调用结果
    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (get_fri_response.rscode().err_code() == 0)
        {
            std::cout << "rpc GetFriendList response success." << std::endl;
            int size = get_fri_response.friends_size();
            for (int i = 0; i < size; i++)
            {
                std::cout << "Index:" << i << " name: " << get_fri_response.friends(i) << std::endl;
            }
        }
        else
        {
            controller.SetFailed("rpc GetFriendList response error");
        }
    }

    return 0;
}