#pragma once
#include <functional>
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "EventLoop.h"
#include <memory>

class Acceptor
{
private:
    // Acceptor对应的事件循环，在构造函数中传入
    const std::unique_ptr<EventLoop> &loop_;   // Acceptor对loop_没有所有权，不能使用移动语义，只能使用重引用

    // 服务端用于监听的Socket，在构造函数中创建
    Socket servsock_; 

    // Acceptor对应的channel，在构造函数中创建
    Channel acceptchannel_; 

    //将指向TcpServer::newconnection()
    std::function<void(std::unique_ptr<Socket>)> newconnection_;
public:
    Acceptor(const std::unique_ptr<EventLoop> &loop, const std::string &ip, const uint16_t port);
    ~Acceptor();

    // 处理新客户端连接请求。
    void newconnection();    

    //设置处理新客户端连接请求的回调函数
    void setnewconnectioncb(std::function<void(std::unique_ptr<Socket>)> fn);  
};
