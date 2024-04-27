#pragma once
#include <functional>
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "EventLoop.h"

class Acceptor
{
private:
    // Acceptor对应的事件循环，在构造函数中传入
    EventLoop *loop_;   

    // 服务端用于监听的Socket，在构造函数中创建
    Socket *servsock_; 

    // Acceptor对应的channel，在构造函数中创建
    Channel *acceptchannel_; 

    //将指向TcpServer::newconnection()
    std::function<void(Socket*)> newconnection_;
public:
    Acceptor(EventLoop *loop, const std::string &ip, const uint16_t port);
    ~Acceptor();

    // 处理新客户端连接请求。
    void newconnection();    

    //设置处理新客户端连接请求的回调函数
    void setnewconnectioncb(std::function<void(Socket*)> fn);  
};
