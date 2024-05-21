// 封装网络服务类

#pragma once
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Acceptor.h"
#include "Connection.h"
#include "ThreadPool.h"
#include <map>
#include <mutex>
#include <memory>

class TcpServer
{
private:

    //主事件循环
    std::unique_ptr<EventLoop> mainloop_;         

    // 存放从事件循环的容器
    std::vector<std::unique_ptr<EventLoop>> subloops_;

    //线程池的大小 也就是从事件循环
    int threadnum_;

    //线程池
    ThreadPool threadpool_;

    //一个网络服务端只有一个监听socket，因此一个TcpServer类只有一个Acceptor对象
    Acceptor acceptor_;    

    // 一个TcpServe有多个Connection对象，将其存放在map容器中
    std::map<int, spConnection> conns_;

    // 保护conns_的互斥锁
    std::mutex mmutex_;

    /*回调函数对象， 负责回调EchoServer类中的函数*/
    std::function<void(spConnection)> newconnectioncb_;
    std::function<void(spConnection)> closeconnectioncb_;
    std::function<void(spConnection)> errorconnectioncb_;
    std::function<void(spConnection, std::string &message)> onmessagecb_; 
    std::function<void(spConnection)> sendcompletecb_;
    std::function<void(EventLoop*)> timeoutcb_;



public:
    TcpServer(const std::string &ip, const uint16_t port, int threadnum);
    ~TcpServer();

    // 运行事件循环。
    void start();         

    //停止IO线程和事件循环
    void stop(); 

    void newconnection(std::unique_ptr<Socket> clientsock);

    //关闭客户端的连接， 在Connection类中回调这个函数
    void closeconnection(spConnection conn);

    //客户端连接发生错误， 在Connection类中回调这个函数
    void errorconnection(spConnection conn);

    // 处理客户端的请求报文， 在Connection中回调这个函数
    void onmessage(spConnection conn, std::string &message);  

    // 数据发送完成后，在Connection类中回调此函数
    void sendcomplete(spConnection conn);

    // epoll_wait()超时，在EventLoop类中回调这个函数
    void epolltimeout(EventLoop* loop);

    /*设置EchoServer回调函数*/
    void newconnectioncb(std::function<void(spConnection)> fn);
    void closeconnectioncb(std::function<void(spConnection)> fn);
    void errorconnectioncb(std::function<void(spConnection)> fn);
    void onmessagecb(std::function<void(spConnection, std::string &message)> fn);
    void sendcompletecb(std::function<void(spConnection)> fn);
    void timeoutcb(std::function<void(EventLoop*)> fn);

    //删除conns_中的Connection对象，在EventLoop::handletimer()中将回调此函数
    void removeconn(int fd);

};