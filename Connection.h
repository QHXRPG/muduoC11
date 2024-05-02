#pragma once
#include <functional>
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Buffer.h"
#include <memory>
#include <atomic>

class Connection;
using spConnection = std::shared_ptr<Connection>;

class Connection:public std::enable_shared_from_this<Connection>
{
private:
    // Acceptor对应的事件循环，在构造函数中传入
    const std::unique_ptr<EventLoop> &loop_;   

    // 服务端用于监听的Socket，生命周期由Connection管理，需要用智能指针包装，在构造函数中创建
    std::unique_ptr<Socket> clientsock_; 

    // Acceptor对应的channel，在构造函数中创建
    std::unique_ptr<Channel> clientchannel_; 

    // 关闭fd_的回调函数。 将回调TcpServe::closeconnection()
    std::function<void(spConnection)> closecallback_;

    // fd_错误的回调函数。 将回调TcpServe::errorconnection()
    std::function<void(spConnection)> errorcallback_;

    // 报文处理的回调函数。将回调TcpServe::onmessage()
    std::function<void(spConnection, std::string&)> onmessagecallback_;

    // 数据发送完之后的回调函数，将回调TcpServe::sendcomplete()
    std::function<void(spConnection)> sendcompletecallback_;

    // 接收缓冲区
    Buffer inputbuffer_;

    // 发送缓冲区
    Buffer outputbuffer_;

    //如果TCP连接断开，就不发送数据了,这个时判断客户端是否断开
    std::atomic_bool disconnect_; // 在IO线程中会改变这个线程的值，在工作线程中会判断这个线程的值，需要进行原子定义




public:
    Connection(const std::unique_ptr<EventLoop> &loop, std::unique_ptr<Socket> clientsock);
    ~Connection();

    int fd() const;                              // 返回fd_成员。
    std::string ip() const;
    uint16_t port() const;

    //TCP连接断开的回调函数  供Channel回调
    void closecallback();

    //TCP连接错误的回调函数  供Channel回调
    void errorcallback();

    //设置fd_关闭的回调函数，
    void setclosecallback( std::function<void(spConnection)> fn);

    //设置fd_发生错误的回调函数
    void seterrorcallback( std::function<void(spConnection)> fn);

    //设置报文处理的回调函数
    void setonmessagecallback(std::function<void(spConnection, std::string&)> fn);

    //设置发送完成的回调函数
    void setsendcompletecallback(std::function<void(spConnection)> fn);

    // 处理写事件的回调函数，供Channel回调
    void writecallback();

    //处理对端发送过来的数据
    void onmessage();

    void send(const char *data, size_t size);
};