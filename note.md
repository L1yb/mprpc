protobuf
## 下载安装
安装命令:
```bash
sudo apt-get install autoconf automake libtool make g++ unzip
```
安装步骤:

下载源码 发布版地址
以c++为例，下载最新的发布版 `protobuf-cpp-3.14.0.zip`
```bash
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.14.0/protobuf-cpp-3.14.0.zip
```
```bash
# 解压压缩包:
unzip protobuf-cpp-3.14.0.zip
# 到源码目录.
cd ~/protobuf-3.14.0
# 生成配置文件:
./autogen.sh
# 配置环境:
./configure
# 编译源码:
make
# 安装:
sudo make install
# 刷新动态库:
sudo ldconfig
```


## 编译
1. 编译proto文件
```bash
protoc test.proto --cpp_out=./
```
2. 编译main.cc
```bash
g++ main.cc test.pb.cc -o test_proto -lprotobuf -std=c++11 -lpthread

```

- `main.cc` 为需要编译的文件
- `test.pb.cc` 是proto生成的cpp文件
- `-0` 指定名称
- `-lprotobuf` 是连接库
- `-std=c++11 -lpthread` 不加可能会报错，我测试没报错



## 简单的test 以及其他操作
```proto
// test.proto
syntax = "proto3"; //声明protobuf版本

package fixbug; //声明包名, 对c++来说是namespace

message ResponseMsg {
    int32 error_code = 1;
    bytes err_msg = 2;
}

//定义登陆消息类型 name pwd
message LoginRequest {
    bytes name = 1;
    bytes pwd = 2;
}

//定义消息相应消息类型
message LoginResponse {
    ResponseMsg rmsg = 1;
    bool success = 2; 
}

//列表数据
message GetFriendListRequest {
    uint32 userid = 1;
} 
message User {
    bytes name = 1;
    uint32 age = 2;
    enum Sex {
        MAN = 0;
        WOMAN = 1;
    }
    Sex sex = 3;
}

message GetFriendListResponse {
    ResponseMsg rmsg = 1;
    repeated User friend_list = 2; //定义列表类型 repeated
}


```

定义列表类型的时候使用`repeated`关键字，可以重复任意次，`.pb.h`中会生成相关函数方法：
| 函数 | 作用 |
-|-
int friend_list_size() const | 获取创建的列表对象数量 
void clear_friend_list() | 清空列表 
User* add_friend_list() | 创建User对象的指针，添加列表元素 
User& friend_list(int index) const| 获取列表索引的引用，不可修改 
User* mutable_friend_list(int index) | 获取索引对象的指针，可修改 

使用实例 
```cpp
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
```


```cpp
// main.cc
#include "test.pb.h"
#include <iostream>
#include <string>
using namespace fixbug;

int main() {
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
```
相当于创建一个cpp的类，name 和 pwd相当于成员变量
> 不知道如何使用成员函数可以查看定义，`SerializeToString`定义的返回值为bool，所以可以当作判断条件看是否序列化成功，函数的参数列表为`std::string* output` ,序列化成功会写入output，所以创建string对象并传递引用


## 描述rpc方法
```proto
//定义option选项生成service服务类和rpc方法描述，默认不生成
option cc_generic_services = true;
//描述rpc方法
service UserServiceRpc
{
    rpc Login(LoginRequest) returns(LoginResponse);
    rpc GetFriendList(GetFriendListRequest) returns(GetFriendListResponse);
}
```
|类|继承|
---|---
class GetFriendListResponse final | public ::google::protobuf::Message
class UserServiceRpc | public ::google::protobuf::Service 
class UserServiceRpc_Stub | public UserServiceRpc

这里提到了两个protobuf的两个基类`Message` `Service`
**Callee ServiceProvider rpc服务提供者**
`UserServiceRpc`生成的类声明了三个方法
```cpp

  static const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* descriptor();

  virtual void Login(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done);
  virtual void GetFriendList(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::fixbug::GetFriendListRequest* request,
                       ::fixbug::GetFriendListResponse* response,
                       ::google::protobuf::Closure* done);
```
| 函数 | 参数列表 | 作用 |
--- | --- | ---
virtual void Login |google::protobuf::RpcController* controller | 
virtual void GetFriendList |const ::fixbug::xxxRequest* request |
 |::fixbug::xxxResponse* response |
 |::google::protobuf::Closure* done | 
static const ::google::protobuf::ServiceDescriptor* descriptor | | 描述上面的方法

**Caller ServiceProvider rpc服务消费者**
`UserServiceRpc_Stub` 负责**序列化、反序列化**相关操作，类中同样继承并且重写的`Login` 和`GetFriendList`，不同是有一个channel函数和私有成员变量
```cpp
  inline ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel() { return channel_; }
 private:
  ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel_;
```
在定义中,实现部分最终都调用了channel的CallMethod方法，而这个方法从何而来
```cpp
void UserServiceRpc_Stub::Login(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                              const ::fixbug::LoginRequest* request,
                              ::fixbug::LoginResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(0),
                       controller, request, response, done);
}
```
`UserServiceRpc_Stub`中的`channel_`成员调用的却是 `UserServiceRpc` 的 `CallMethod` 方法？为什么呢？来看看 `RpcChannel` 的定义

```cpp
  virtual void CallMethod(const MethodDescriptor* method,
                          RpcController* controller, const Message* request,
                          Message* response, Closure* done) = 0;

```
`CallMethod` 是一个纯虚函数，因此 `RpcChannel` 是一个抽象基类， 他无法创建对象，想要使用 `CallMethod`只能创建抽象类覆盖此函数，所以我们可以看到：`UserServiceRpc_Stub`构造函数必须要求传入基类指针，而**基类指针是可以接受派生类对象的**，所以就解释通了
```cpp
UserServiceRpc_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel);
```
### !CallMethod调用解释
`UserServiceRpc_Stub`在构造的时候传入一个`RpcChannel`的派生类对象初始化 `channel_`，~~而`CallMethod` 被 `UserServiceRpc` 覆盖了，`UserServiceRpc_Stub`继承了`UserServiceRpc`，因此出现了子类调用~~
> ~~但是我发现 这里覆盖的`CallMethod`，并不是`RpcChannel`的方法，而是 `Service`的方法~~
> ~~chat提到名称隐藏，指的是两条继承链上重名函数但不影响，或派生类与基类成员名称相同会隐藏基类中的成员~~
> ~~如果这是个问题的话，解决方法就是明确指定作用域~~

> 正解，这个`CallMethod`其实是`RpcChannel`的派生类重定义的（天才wyq）



## userservice 用户服务端
```cpp
    //框架初始化
    MprpcApplication::Init(argc, argv);
    //创建节点
    RpcProvider provider;
    //将UserService发布到节点上
    provider.NotifyService(new UserService());
    //运行节点
    provider.Run();
```
### userservice类定义
重写基类虚函数
    下面的方法是框架调用的
    远程传来一个请求request，包含了数据信息，本地获取信息来做业务bool Login
    然后填写response消息，返回业务结果，回调
    void login()在基类中是虚函数，不知道我们会有什么样的业务，所以这里需要重写

本地业务重写的四个步骤：
1. 框架给业务上报了请求参数，应用获取参数来做本地业务
2. 做本地业务
3. 写入反馈相应，包含错误码、错误消息、返回值
4. 执行回调 执行消息返回过程中的序列化与网络发送（都是由框架完成的）
```cpp
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "local service: Login." << std::endl;
        std::cout << "name: " << name << " pwd: " << pwd << std::endl;
        return true;
    }
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

        // 4. 执行回调,执行消息返回过程中的序列化与网络发送（都是由框架完成的）
        done->Run();

    }
```


### RpcApplication 定义使用方法
框架的基类，负责框架的初始化
#### Init方法
    程序启动的时候一般都会带参数， Init 需要解析这些参数
```cpp
void MprpcApplication::Init(int argc, char** argv)
{
    if (argc < 2)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c = 0;
    std::string config_file;
    while ((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            std::cout << "invalid args!" << std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            std::cout << "need <configfile>!" << std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }
}
```
#### ！使用单例模式的思想
但是这种但是模式是线程不安全的


### *RpcProvider 定义使用方法
p23前两分钟梳理的流程多看两遍
总体思路：
这一步是位于muduo接受到网络传输过来的请求后的反序列化，交给本地业务之后，序列化回调响应
首先是**获取**外部的service信息，建表**存储**所有请求的服务和包含的方法（NotifyService），然后运行节点**创建**Tcpserver对象，绑定连接和消息的回调，在消息回调中获取请求的字符流，**反序列化**，得到需要**调用**服务和方法，生成**响应**，**系列化响应并发送**，结束后主动**断开网络服务**


#### NotifyService方法
实现功能：
- 获取服务的描述信息
- 使用无序表记录有哪些对象， 包含哪些方法
- 这是框架提供给**外部**使用的，可以发布rpc方法的函数**接口**


#### Run 方法
**启动rpc发布节点，阻塞线程，等待rpc调用请求**
```cpp
  TcpServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg,
            Option option = kNoReusePort);
```
1. 创建TcpServer 对象需要四个参数,因此产生的操作
- 创建`muduo::net::EventLoop m_eventloop`
- 提供网络的地址
    - ip
    - port

2. 绑定连接回调和消息读写回调
分离了业务代码和网络代码
```cpp
server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, 
                                            std::placeholders::_2, std::placeholders::_3));

```
3. 设置muduo库线程数
4. 启动网络服务

#### OnMessage
已建立连接用户的读写时间回调
如果远端发送了一个rpc服务的调用请求，那么 OnMessage方法就会响应
在框架内部，RpcProvider和RpcComsumer协商好之间通信使用的protobuf数据类型
应包含 service_name method_name args_size
定义proto的message类型，进行数据头的系列化和反序列化
UserServiceLoginzhangsan123456
header_size(四个字节)   header_str   args_str
std::string insert和copy方法
```cpp
void UserServiceRpc::CallMethod(const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method,
                             ::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                             const ::PROTOBUF_NAMESPACE_ID::Message* request,
                             ::PROTOBUF_NAMESPACE_ID::Message* response,
                             ::google::protobuf::Closure* done)
```
1. 网络上接受远程rpc调用请求的字符流 
2. 从字符流中读取四个字节的内容
3. 根据 header_size 读取原始数据流， 反序列化数据， 得到rpc请求的详细信息
4. 获取service对象和 method 对象
5. 生成rpc方法调用的请求request和响应 response
6. 给下面方法调用 绑定一个closure回调
7. 在框架上根据远程rpc请求，调用当前rpc节点上发布的方法

#### SendRpcResponse
Closure回调操作，用于序列化响应和网络发送



## 日志系统
RpcProvider 使用epoll和多线程 因此会有多个线程写日志信息， 我们的**queue必须保证线程安全**， queue做磁盘I/O写入log.txt，写日志线程在queue为空的时候，不必参与抢夺互斥锁（条件变量）
（线程间通信）
每天更新日志，每个日志不超过20M



### kafka

### linux 里的线程安全方法




## 基类、派生类、（纯）虚函数
类的继承构成一种层次关系，在根部的是**基类**，直接或者间接从基类继承来的叫**派生类**
虚函数：定义在基类中，基类希望派生类自定义适合自己的版本，于是声明为虚函数，派生类**必须重新声明所有虚函数**、
- 当使用基类的引用或指针调用一个虚函数时将发动动态绑定
- 动态绑定理解为运行时绑定， 一个函数参数列表传入基类对象的**引用**，其运行版本将由运行时具体对象决定,可能是基类版本，也可能是派生类版本
- 基类通常定义一个虚析构函数
- 派生类并不是一定要覆盖继承的虚函数，在覆盖前也不是一定要加virtual关键字
    - 如果覆盖了，那么形参列表和返回值必须严格匹配（注意并没有要求完全一致，有例外，不做了解）
    - 覆盖时加override关键字，可以在形参不同时报错
    - final不允许后续再进行覆盖
纯虚函数：在虚函数声称后加“ = 0”（只能出现在**类内虚函数声明出**），就可以说明为纯虚函数
- 纯虚函数可以有定义，但是函数体必须在函数外部
- **含有（或者未经覆盖直接继承）纯虚函数的类是抽象基类**
- 抽象基类负责定义接口，抽象基类不能创建对象(因为包含纯虚函数，意味着没有完整的定义)，需要定义派生类来覆盖纯虚函数，才可以定义对象

## 单例模式

## 类外实现static函数
类外实现static函数不用带static

## [bind 函数绑定](https://blog.csdn.net/qq_38410730/article/details/103637778)
头文件 `<functional>`
`std::bind` 是一个函数适配器：**接受一个可调用对象，生成一个新的可调用对象来适用原对象的参数列表**
作用：
- 将可调用对象和其参数绑定成一个仿函数；
- 只绑定部分参数，减少可调用对象传入的参数。

一般的表达形式:
```cpp
auto newCallable = bind(callable, arg_list);
```
该形式表达的意思是：当调用newCallable时，会调用callable，并传给它arg_list中的参数
```cpp
// 普通函数绑定
int fun_1(int a, int b, int c)
{
    ...
}
-------------------------------
auto f1 = std::bind(fun_1,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
f1(1, 2, 3) // a = 1, b = 2, c = 3;
auto f1 = std::bind(fun_1,std::placeholders::_1, std::placeholders::_2, 10);
f1(1, 2) //a = 1, b = 2, c = 10 减少传入的参数
// 成员函数绑定
class A
{
public:
    int func_2(int a, int b)
    {
        ...
    }
}
--------------------------
A a;
auto f2 = std::bind(&A::func_2, &a, std::placeholders::_1,10);
f2(1); // a = 1, b =10


```
对于成员函数的绑定，第一个是成员函数的指针，**因为编译器不会隐式转换成员函数名为函数指针，所以需要&显式的转化**，所以，**编译器会隐式转换普通函数名为函数指针**
第二个参数是**对象的指针**



## CMakeLsits 静态库 动态库 依赖

## gdb 简单调试

## find 命令
```bash
find [路径] [匹配条件] [动作]
```
1. 路径 是要查找的目录路径，可以是一个目录或文件名，也可以是多个路径，多个路径之间用空格分隔，如果未指定路径，则默认为当前目录。

2. expression 是可选参数，用于指定查找的条件，可以是文件名、文件类型、文件大小等等。
匹配条件 中可使用的选项有二三十个之多，以下列出最常用的部份：

- name pattern：按文件名查找，支持使用通配符 * 和 ?。
- type type：按文件类型查找，可以是 f（普通文件）、d（目录）、l（符号链接）等。
- size [+-]size[cwbkMG]：按文件大小查找，支持使用 + 或 - 表示大于或小于指定大小，单位可以是 c（字节）、w（字数）、b（块数）、k（KB）、M（MB）或 G（GB）。
- mtime days：按修改时间查找，支持使用 + 或 - 表示在指定天数前或后，days 是一个整数表示天数。
- user username：按文件所有者查找。
- group groupname：按文件所属组查找。
3. 动作: 可选的，用于对匹配到的文件执行操作，比如删除、复制等。

find 命令中用于时间的参数如下：

- amin n：查找在 n 分钟内被访问过的文件。
- atime n：查找在 n*24 小时内被访问过的文件。
- cmin n：查找在 n 分钟内状态发生变化的文件（例如权限）。
- ctime n：查找在 n*24 小时内状态发生变化的文件（例如权限）。
- mmin n：查找在 n 分钟内被修改过的文件。
- mtime n：查找在 n*24 小时内被修改过的文件。
查找当前目录下名为 file.txt 的文件：
find . -name file.txt

将当前目录及其子目录下所有文件后缀为 .c 的文件列出来:
find . -name "*.c"

将当前目录及其子目录中的所有文件列出：
 find . -type f

查找 /home 目录下大于 1MB 的文件：
find /home -size +1M

查找 /var/log 目录下在 7 天前修改过的文件：
find /var/log -mtime +7

查找过去 7 天内被访问的文件:
find /path/to/search -atime -7

## 命令行参数解析
```cpp
int getopt(int argc, char * const argv[], const char *optstring);
```
- argc：命令行参数的数量。
- argv：命令行参数的字符串数组。
- optstring： 包含合法选项字符的字符串。字符后面跟 : 表示该选项需要一个参数。
假设有这么一个输入：
```bash
./your_program -i config.txt
```
getopt会解析命令，如果包含“-i”将会返回i，如果需要参数（例如 `-i <configfile>`），getopt会将这个值保存在optarg中,如果提供无效的选项，将会返回‘?’,如果需要参数但是缺少参数将会返回‘：’
`exit(EXIT_FAILURE)` 会终止程序并返回状态代码
    - EXIT_FAILURE	1
    - EXIT_SUCCESS	0

## 字符串组装 sprintf snprintf 
```cpp
#include <cstdio>

int main() {
    char buffer[10];
    sprintf(buffer, "Number: %d", 12345);
    printf("%s\n", buffer);  // 可能导致缓冲区溢出

    char buffer[10];
    snprintf(buffer, sizeof(buffer), "Number: %d", 12345);
    printf("%s\n", buffer);  // 会被安全截断为 "Number: 1"
    return 0;
}

```

### **总结表格**
| 特性           | `sprintf`                     | `snprintf`                   |
|----------------|--------------------------------|-------------------------------|
| **缓冲区大小检查** | 无，可能溢出                 | 有，安全                      |
| **返回值**      | 写入字符数（不包括 `\0`）      | 实际需要的字符数              |
| **安全性**      | 较低                          | 高                           |
| **推荐场景**    | 缓冲区大小固定且非常确定       | 不确定缓冲区大小或动态数据   |

**推荐始终使用 `snprintf` 来确保代码的安全性和健壮性。**
