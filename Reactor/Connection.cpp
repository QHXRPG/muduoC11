#include "Connection.h"


Connection::Connection(EventLoop *loop, std::unique_ptr<Socket> clientsock)
:loop_(loop),
clientsock_(std::move(clientsock)),  // unqiue赋值函数已被输出，要使用移动语义
disconnect_(false),
clientchannel_(new Channel(loop_, clientsock_->fd()))
{
    // 为新客户端连接准备读事件，并添加到epoll中。
    clientchannel_->setreadcallback(std::bind(&Connection::onmessage, this));
    clientchannel_->setclosecallback(std::bind(&Connection::closecallback, this));
    clientchannel_->seterrorcallback(std::bind(&Connection::errorcallback, this));
    clientchannel_->setwritecallback(std::bind(&Connection::writecallback, this));
    clientchannel_->useet();                 // 客户端连上来的fd采用边缘触发。 
    clientchannel_->enablereading();      // 让epoll_wait()监视clientchannel的读事件
}

Connection::~Connection()
{
}

int Connection::fd() const                              // 返回fd_成员。
{
    return clientsock_->fd();
}
std::string Connection::ip() const
{
    return clientsock_->ip();
}
uint16_t Connection::port() const
{
    return clientsock_->port();
}

//TCP连接断开的回调函数
void Connection::closecallback()
{
    disconnect_ = true;
    clientchannel_->remove();  //从事件循环中删除channel
    closecallback_(shared_from_this());            // 关闭客户端的fd。
}

//TCP连接错误的回调函数
void Connection::errorcallback()
{
    disconnect_ = true;
    clientchannel_->remove();  //从事件循环中删除channel
    errorcallback_(shared_from_this());            // 关闭客户端的fd。
}


//设置fd_关闭的回调函数，
void Connection::setclosecallback( std::function<void(spConnection)> fn)
{
    closecallback_ = fn;
}

//设置fd_发生错误的回调函数
void Connection::seterrorcallback( std::function<void(spConnection)> fn)
{
    errorcallback_ = fn;
}

void Connection::onmessage()
{
    char buffer[1024];
    while (true)             // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕。
    {    
        bzero(&buffer, sizeof(buffer));
        ssize_t nread = read(fd(), buffer, sizeof(buffer));
        if (nread > 0)      // 成功的读取到了数据。
        {
            // 把读取的数据追加到接收缓冲区中
            inputbuffer_.append(buffer, nread); 
        } 
        else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
        {  
            continue;
        } 
        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
        {
            std::string message;
            while (1)
            {
                // 如果message为空或者不完整，退出循环并继续等待
                if(inputbuffer_.pickmessage(message) == false) break; 

                //更新时间戳
                lastatime_ = Timestamp::now();

                /*业务代码*/
                onmessagecallback_(shared_from_this(), message);  //回调TcpServe::onmessage()
            }
            break;
        } 
        else if (nread == 0)  // 客户端连接已断开。
        {   
            closecallback();
            break;
        }
    }
}

void Connection::setonmessagecallback(std::function<void(spConnection, std::string&)> fn)
{
    onmessagecallback_ = fn;
}

// 发送数据，不管在任何线程中，都是调用此函数发送数据
void Connection::send(const char *data, size_t size)
{
    if(disconnect_ == true)
    {
        return;
    }
    // 因为数据要发给其它线程处理，所以，把它包装成智能指针
    std::shared_ptr<std::string> message(new std::string(data));

    // 判断当前线程是否为事件循环线程（IO线程）
    if(loop_->isinloopthread())
    {
        // 如果当前线程时IO线程， 直接 执行 发送数据的操作
        sendinloop(message);
    }
    else
    {
        // 如果当前线程不是 IO线程， 把发送数据的操作交给IO线程去执行
        loop_->queueinloop(std::bind(&Connection::sendinloop, this, message));
    }
}

// 发送数据，如果当前线程是IO线程，直接调用此线程，如果是工作线程，将此函数传给IO线程
void Connection::sendinloop(std::shared_ptr<std::string> data)
{
    //把数据发送到Connection的发送缓冲区中
    outputbuffer_.appendwithsep(data->data(), data->size());  

    //注册写事件
    clientchannel_->enablewriting();
}

void Connection::writecallback()
{
    // 尝试把发送缓冲区的数据全部发送出去
    int writen = ::send(fd(), outputbuffer_.data(), outputbuffer_.size(), 0);  

    if(writen > 0)
        outputbuffer_.erase(0, writen);  // 如果大于0，在发送缓冲区中删除发送成功的字节数

    if(outputbuffer_.size() == 0)  // 如果发送缓冲区中没有数据，不再关注写事件
    {
        clientchannel_->disablewriting();
        sendcompletecallback_(shared_from_this());
    }

}

//设置发送完成的回调函数
void Connection::setsendcompletecallback(std::function<void(spConnection)> fn)
{
    sendcompletecallback_ = fn;
}

// 判断TCP连接是否超时（空闲太久）
bool Connection::timeout(time_t now, int val)
{
     return now - lastatime_.toint() > val;  //如果大于val秒，则超时
} 