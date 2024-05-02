// 回显业务,顶层
#pragma once
#include "TcpServer.h"
#include "EventLoop.h"
#include "Connection.h"
#include "ThreadPool.h"

class EchoServer
{
private:
    TcpServer tcpserver_;
    ThreadPool threadpool_;   // 工作线程池

public:
    EchoServer(const std::string &ip,const uint16_t port, int subThreadnum=3, int workthreadnum=5);
    ~EchoServer();

    // 运行事件循环。
    void Start();          

    void HandleNewConnection(spConnection conn);

    //关闭客户端的连接， 在TcpServer类中回调这个函数
    void HandleClose(spConnection conn);

    //客户端连接发生错误， 在TcpServer类中回调这个函数
    void HandleError(spConnection conn);

    // 处理客户端的请求报文， 在TcpServer中回调这个函数
    void HandleMessage(spConnection conn, std::string &message);  

    // 数据发送完成后，在TcpServer类中回调此函数
    void HandleSendComplete(spConnection conn);

    // epoll_wait()超时，在TcpServer类中回调这个函数
    void HandleTimeout(EventLoop *loop);

    // 处理客户端的请求报文，用于添加给线程池
    void OnMessage(spConnection conn, std::string &message);
};