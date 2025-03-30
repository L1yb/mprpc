#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include <vector>
#include "logger.h"
class FriendService : public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendList(uint32_t id)
    {
        std::cout << "do local service FriendService. getFriendList method" << std::endl;
        std::vector<std::string> friendList;
        friendList.push_back("hello hello");
        friendList.push_back("world world");
        return friendList;
    }

    void GetFriendList(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendListRequest* request,
                       ::fixbug::GetFriendListResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t userid = request->id();
        std::cout << "user id: " << userid << std::endl;
        std::vector<std::string> friendList = GetFriendList(userid);
        response->mutable_rscode()->set_err_code(0);
        response->mutable_rscode()->set_err_msg("");
        for (int i = 0; i < friendList.size(); i++) 
        {
            response->add_friends(friendList[i]);
        }

        done->Run();
    }

};



int main(int argc, char** argv)
{   
    // LOG_INFO("first log message.");
    // LOG_ERR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);
    //框架初始化
    MprpcApplication::Init(argc, argv);
    //创建节点
    RpcProvider provider;
    //将UserService发布到节点上
    provider.NotifyService(new FriendService());
    //运行节点
    provider.Run();


    return 0;
}