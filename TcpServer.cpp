#include "TcpServer.h"

TcpServer::TcpServer(const std::string &ip,const uint16_t port,int threadnum)
:threadnum_(threadnum),
mainloop_(new EventLoop(true)), // 创建主事件循环。
acceptor_(mainloop_.get(), ip, port), //创建监听socket
threadpool_(threadnum_, "IO") // 创建线程池。
{
    // 设置timeout超时的回调函数。
    mainloop_->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));   

    // 指定Acceptor为主事件循环，用来监听和建立连接
    acceptor_.setnewconnectioncb(std::bind(&TcpServer::newconnection, this, std::placeholders::_1));

    // 创建从事件循环。
    for (int ii=0;ii<threadnum_;ii++)
    {
        // 创建从事件循环，存入subloops_容器中。
        subloops_.emplace_back(new EventLoop(false, 5, 10));  

        // 设置timeout超时的回调函数。
        subloops_[ii]->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));   

        // 设置timeout超时的回调函数
        subloops_[ii]->settimercallback(std::bind(&TcpServer::removeconn, this, std::placeholders::_1));

        // 在线程池中运行从事件循环。
        threadpool_.addtask(std::bind(&EventLoop::run, subloops_[ii].get()));    
    }
}

TcpServer::~TcpServer()
{

}

// 运行事件循环。
void TcpServer::start()          
{
    mainloop_->run();
}

//停止IO线程和事件循环
void TcpServer::stop()
{
    // 停止主从事件循环
    mainloop_->stop();
    printf("主事件循环已停止\n");

    for(int ii=0; ii<threadnum_; ii++)
    {
        subloops_[ii]->stop();
    }
    printf("从事件循环已停止\n");

    // 停止IO线程
    threadpool_.stop();
    printf("IO线程池停止\n");
}

// 处理新客户端连接请求。
void TcpServer::newconnection(std::unique_ptr<Socket> clientsock)
{
    // 把新建的conn分配给从事件循环。将不同的客户端连接均匀地分配到不同的事件循环中,实现负载均衡
    spConnection conn(new Connection(subloops_[clientsock->fd()%threadnum_].get(), std::move(clientsock)));   
    conn->setclosecallback(std::bind(&TcpServer::closeconnection, this, std::placeholders::_1));
    conn->seterrorcallback(std::bind(&TcpServer::errorconnection, this, std::placeholders::_1));
    conn->setonmessagecallback(std::bind(&TcpServer::onmessage, this, std::placeholders::_1, std::placeholders::_2));
    conn->setsendcompletecallback(std::bind(&TcpServer::sendcomplete, this, std::placeholders::_1));
    
    {
        std::lock_guard<std::mutex> gd(mmutex_);
        // 把conn存放在 TcpServe 的 map容器中。
        conns_[conn->fd()]=conn;  
    }
  

    // 把conn存放在 EventLoop 的 map容器中。
    subloops_[conn->fd()%threadnum_]->newconnection(conn);

    // 回调EchoServer::HandleNewConnection()。
    if (newconnectioncb_) newconnectioncb_(conn);             
}

 // 关闭客户端的连接，在Connection类中回调此函数。 
 void TcpServer::closeconnection(spConnection conn)
 {
    // 回调EchoServer::HandleClose()。
    if (closeconnectioncb_) closeconnectioncb_(conn);

    {
        std::lock_guard<std::mutex> gd(mmutex_);
        conns_.erase(conn->fd());        // 从map中删除conn。
        //  系统会自动释放Connection对象。
    }
 }

// 客户端的连接错误，在Connection类中回调此函数。
void TcpServer::errorconnection(spConnection conn)
{
     // 回调EchoServer::HandleError()。
    if (errorconnectioncb_) errorconnectioncb_(conn);    

    {
        std::lock_guard<std::mutex> gd(mmutex_);
        conns_.erase(conn->fd());      // 从map中删除conn。
        //  系统会自动释放Connection对象。
    }
}

// 处理客户端的请求报文，在Connection类中回调此函数。
void TcpServer::onmessage(spConnection conn,std::string& message)
{
    // 回调EchoServer::HandleMessage()。
    if (onmessagecb_) onmessagecb_(conn,message);     
}

// 数据发送完成后，在Connection类中回调此函数。
void TcpServer::sendcomplete(spConnection conn)     
{
    if (sendcompletecb_) sendcompletecb_(conn);     // 回调EchoServer::HandleSendComplete()。
}

// epoll_wait()超时，在EventLoop类中回调此函数。
void TcpServer::epolltimeout(EventLoop *loop)         
{
    if (timeoutcb_)  timeoutcb_(loop);           // 回调EchoServer::HandleTimeOut()。
}

void TcpServer::newconnectioncb(std::function<void(spConnection)> fn)
{
    newconnectioncb_=fn;
}

void TcpServer::closeconnectioncb(std::function<void(spConnection)> fn)
{
    closeconnectioncb_=fn;
}

void TcpServer::errorconnectioncb(std::function<void(spConnection)> fn)
{
    errorconnectioncb_=fn;
}

void TcpServer::onmessagecb(std::function<void(spConnection,std::string &message)> fn)
{
    onmessagecb_=fn;
}

void TcpServer::sendcompletecb(std::function<void(spConnection)> fn)
{
    sendcompletecb_=fn;
}

void TcpServer::timeoutcb(std::function<void(EventLoop*)> fn)
{
    timeoutcb_=fn;
}

//删除conns_中的Connection对象，在EventLoop::handletimer()中将回调此函
void TcpServer::removeconn(int fd)
{
    {
        std::lock_guard<std::mutex> gd(mmutex_);
        // 从map中删除conn
        conns_.erase(fd);
    }
}