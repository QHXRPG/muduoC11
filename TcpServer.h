// 封装网络服务类

#pragma once
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Acceptor.h"
#include "Connection.h"

class TcpServer
{
private:

    // 一个TcpServer可以有多个事件循环，现在是单线程，暂时只用一个事件循环。
    EventLoop loop_;         

    //一个网络服务端只有一个监听socket，因此一个TcpServer类只有一个Acceptor对象
    Acceptor *acceptor_;    
public:
    TcpServer(const std::string &ip, const uint16_t port);
    ~TcpServer();

    void start();          // 运行事件循环。

    void newconnection(Socket *clientsock);
};