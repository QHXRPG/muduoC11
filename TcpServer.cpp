#include "TcpServer.h"

TcpServer::TcpServer(const std::string &ip,const uint16_t port)
{
    acceptor_ = new Acceptor(&loop_, ip, port);
    acceptor_->setnewconnectioncb(std::bind(&TcpServer::newconnection, this, std::placeholders::_1));
}

TcpServer::~TcpServer()
{
    delete acceptor_;
}

// 运行事件循环。
void TcpServer::start()          
{
    loop_.run();
}

// 处理新客户端连接请求。
void TcpServer::newconnection(Socket *clientsock)
{
    // 为新客户端连接准备读事件，并添加到epoll中。
    Connection *conn = new Connection(&loop_, clientsock);
}