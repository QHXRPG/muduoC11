#include "EchoServer.h"



EchoServer::EchoServer(const std::string &ip,const uint16_t port, int subThreadnum, int workthreadnum)
:tcpserver_(ip, port, subThreadnum),
threadpool_(workthreadnum, "WORK")
{
    tcpserver_.newconnectioncb(std::bind(&EchoServer::HandleNewConnection, this, std::placeholders::_1));
    tcpserver_.closeconnectioncb(std::bind(&EchoServer::HandleClose, this, std::placeholders::_1));
    tcpserver_.errorconnectioncb(std::bind(&EchoServer::HandleError, this, std::placeholders::_1));
    tcpserver_.onmessagecb(std::bind(&EchoServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_.sendcompletecb(std::bind(&EchoServer::HandleSendComplete, this, std::placeholders::_1));
    tcpserver_.timeoutcb(std::bind(&EchoServer::HandleTimeout, this, std::placeholders::_1));
}

EchoServer::~EchoServer()
{

}

// 运行事件循环。
void EchoServer::Start()
{
    tcpserver_.start();
}          

void EchoServer::HandleNewConnection(spConnection conn)
{
    std::cout << "New Connection Come in" <<std::endl;
    /*业务代码*/
}

//关闭客户端的连接， 在TcpServer类中回调这个函数
void EchoServer::HandleClose(spConnection conn)
{
    std::cout << "EchoServer conn closed" <<std::endl;
    /*业务代码*/  
}

//客户端连接发生错误， 在TcpServer类中回调这个函数
void EchoServer::HandleError(spConnection conn)
{
    std::cout << "EchoServer conn error" <<std::endl;
    /*业务代码*/  
}

// 处理客户端的请求报文， 在TcpServer中回调这个函数
void EchoServer::HandleMessage(spConnection conn, std::string &message)
{
    // 把业务添加到线程池的任务队列中
    threadpool_.addtask(std::bind(&EchoServer::OnMessage, this, conn, message));
}

// 数据发送完成后，在TcpServer类中回调此函数
void EchoServer::HandleSendComplete(spConnection conn)
{
    std::cout << "Message send complete" <<std::endl;
    /*业务代码*/    
}

// epoll_wait()超时，在TcpServer类中回调这个函数
void EchoServer::HandleTimeout(EventLoop *loop)
{
    std::cout << "EchoServer timeout" <<std::endl;
    /*业务代码*/    
}

// 处理客户端的请求报文，用于添加给线程池
void EchoServer::OnMessage(spConnection conn, std::string &message)
{
    message = "reply" + message;  // 回显业务
    conn->send(message.data(), message.size());  // 把数据发送出去, 在send()函数中调用了拼接数据长度的功
}