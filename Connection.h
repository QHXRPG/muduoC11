#pragma once
#include <functional>
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "EventLoop.h"

class Connection
{
private:
    // Acceptor对应的事件循环，在构造函数中传入
    EventLoop *loop_;   

    // 服务端用于监听的Socket，在构造函数中创建
    Socket *clientsock_; 

    // Acceptor对应的channel，在构造函数中创建
    Channel *clientchannel_; 
public:
    Connection(EventLoop *loop, Socket *clientsock);
    ~Connection();
};