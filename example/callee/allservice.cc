#include <iostream>
#include <string>
#include "all.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include <vector>
#include "logger.h"
class UserService : public fixbug::UserServiceRpc
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "local service: Login." << std::endl;
        std::cout << "name: " << name << " pwd: " << pwd << std::endl;
        return true;
    }
    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "local service: Register." << std::endl;
        std::cout << "id: " << id << " name: " << name << " pwd: " << pwd << std::endl;
        return true;
    }
    void Login(::google::protobuf::RpcController* controller,
               const ::fixbug::LoginRequest* request,
               ::fixbug::LoginResponse* response,
               ::google::protobuf::Closure* done)
    {
        // 1.框架给业务上报了请求参数，应用获取参数来做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();
        // 2.做本地业务
        bool login_result = Login(name, pwd);
        // 3. 写入反馈相应，包含错误码、错误消息、返回值
        fixbug::ResultCode* rscode = response->mutable_rscode();
        rscode->set_err_code(0);
        rscode->set_err_msg("");
        response->set_success(login_result);

        // 4.执行回调 执行消息返回过程中的序列化与网络发送（都是由框架完成的）
        done->Run();
    }
    void Register(::google::protobuf::RpcController* controller,
                const ::fixbug::RegisterRequest* request,
                ::fixbug::RegisterResponse* response,
                ::google::protobuf::Closure* done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool register_result = Register(id, name, pwd);

        fixbug::ResultCode* result = response->mutable_rscode();
        result->set_err_code(0);
        result->set_err_msg("");
        response->set_success(register_result);

        done->Run();


    }
};

class FriendService : public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendList(uint32_t id)
    {
        std::cout << "do local service FriendService. getFriendList method" << std::endl;
        std::vector<std::string> friendList;
        friendList.push_back("hello");
        friendList.push_back("world");
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
    //发布 UserService 服务
    provider.NotifyService(new UserService());
    //发布 FriendService 服务
    provider.NotifyService(new FriendService());
    //运行节点
    provider.Run();


    return 0;
}