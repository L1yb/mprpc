#include "test.pb.h"
#include <iostream>
#include <string>
using namespace fixbug;

int main() {
    // LoginResponse rsp;
    // //对象中访问另一个对象时，使用mutable_方法获取对象指针
    // ResponseMsg* rmsg = rsp.mutable_rmsg();
    // rmsg->set_error_code(1);
    // rmsg->set_err_msg("登陆失败.");

    GetFriendListResponse gfrsp;
    ResponseMsg* rmsg = gfrsp.mutable_rmsg();
    rmsg->set_error_code(0);//成功不设置errmsg
    User* user1 =  gfrsp.add_friend_list();
    user1->set_name("TOM");
    user1->set_age(3);
    user1->set_sex(User::MAN);
    User* user2 =  gfrsp.add_friend_list();
    user2->set_name("JERRY");
    user2->set_age(1);
    user2->set_sex(User::MAN);
    std::cout << gfrsp.friend_list_size() << std::endl;
    std::cout << gfrsp.friend_list(1).age() << std::endl;
    User* cur = gfrsp.mutable_friend_list(0);
    cur->set_age(10);
    std::cout << cur->name() << cur->age() << std::endl;
}


int main1() {
    LoginRequest lr;
    lr.set_name("ali");
    lr.set_pwd("123456");
    //序列化
    std::string send_str;
    if (lr.SerializeToString(&send_str)){
        std::cout << send_str << std::endl;
    }
    //反序列化
    LoginRequest lrp;
    if (lrp.ParseFromString(send_str)){
        std::cout << lrp.name() << std::endl;
        std::cout << lrp.pwd() << std::endl;

    }


    return 0;
}