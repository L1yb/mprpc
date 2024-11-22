#include "mprpccontroller.h"

MprpcController::MprpcController()
{
    isFailed = false;
    err_msg = "";
}

void MprpcController::Reset()
{
    isFailed = false;
    err_msg = "";
}
bool MprpcController::Failed() const
{
    return isFailed;

}
std::string MprpcController::ErrorText() const
{
    return err_msg;

}
void MprpcController::SetFailed(const std::string &reason)
{
    isFailed = true;
    err_msg = reason;

}
// 未实现的功能
void MprpcController::StartCancel(){}
bool MprpcController::IsCanceled() const{}
void MprpcController::NotifyOnCancel(google::protobuf::Closure *callback){}
