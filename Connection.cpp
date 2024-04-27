#include "Connection.h"


Connection::Connection(EventLoop *loop, Socket *clientsock)
:loop_(loop),
clientsock_(clientsock)
{
    // 为新客户端连接准备读事件，并添加到epoll中。
    clientchannel_=new Channel(loop_, clientsock_->fd());
    clientchannel_->setreadcallback(std::bind(&Channel::onmessage, clientchannel_));
    clientchannel_->useet();                 // 客户端连上来的fd采用边缘触发。
    clientchannel_->enablereading();   // 让epoll_wait()监视clientchannel的读事件
}

Connection::~Connection()
{
    delete clientchannel_;

    //clientsock_在Channel.cpp中创建，但它的生命周期应该与Connection对象一样
    delete clientsock_;        
}