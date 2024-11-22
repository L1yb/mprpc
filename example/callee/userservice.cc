#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

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
    /*
    重写基类虚函数
    下面的方法是框架调用的
    远程传来一个请求request，包含了数据信息，本地获取信息来做业务
    然后填写response消息，返回业务结果，回调
    login在基类中是虚函数，不知道我们会有什么样的业务，所以这里需要重写
    */
    void Login(::google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done)
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


int main(int argc, char** argv)
{   
    //框架初始化
    MprpcApplication::Init(argc, argv);
    //创建节点
    RpcProvider provider;
    //将UserService发布到节点上
    provider.NotifyService(new UserService());
    //运行节点
    provider.Run();


    return 0;
}