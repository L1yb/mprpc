#pragma once
#include <google/protobuf/service.h>
#include <string>
// #include "mprpcchannel.h"
class MprpcController :public google::protobuf::RpcController
{
public:
    MprpcController();
    void Reset();
    bool Failed() const;
    std::string ErrorText() const;
    void SetFailed(const std::string& reason);
    // 未实现的功能
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);

private:
    std::string err_msg;
    bool isFailed;
};