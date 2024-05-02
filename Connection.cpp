#include "Connection.h"


Connection::Connection(const std::unique_ptr<EventLoop> &loop, std::unique_ptr<Socket> clientsock)
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
            while (1)
            {
                /**************************************************************************************/
                //用指定报文长度的方式解决TCP粘包分包问题
                int len;
                memcpy(&len, inputbuffer_.data(), 4);  // 从接收缓冲区获取报文头部

                //如果接收缓冲区中的数据量小于报文头部，说明inputbuffer中的报文内容不完整
                if(inputbuffer_.size() < len + 4) break;

                //如果报文内容是完整的，取出来
                std::string message(inputbuffer_.data()+4, len);  // 从inputbuffer中获取一段报文 
                inputbuffer_.erase(0, len+4);                     // 删除已获取的报文
                /**************************************************************************************/

                printf("recv(eventfd=%d): %s\n", fd(), message.c_str());

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

void Connection::send(const char *data, size_t size)
{
    if(disconnect_ == true)
    {
        printf("客户端连接已断开，send()直接返回，不发送数据\n");
        return;
    }
    outputbuffer_.appendwithhead(data, size);  //把数据发送到Connection的发送缓冲区中

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