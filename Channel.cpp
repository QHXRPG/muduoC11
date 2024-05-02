#include "Channel.h"

Channel::Channel(const std::unique_ptr<EventLoop> &loop, int fd)
:loop_(loop),
fd_(fd)      // 构造函数。
{

}

// 析构函数。
Channel::~Channel()                            
{
    // 在析构函数中，不要销毁loop_，也不能关闭fd_，
    //因为这两个东西不属于Channel类，Channel类只是需要它们，使用它们而已
}

// 返回fd_成员。
int Channel::fd()                                            
{
    return fd_;
}

// 采用边缘触发。
void Channel::useet()                                    
{
    events_=events_|EPOLLET;
}

// 让epoll_wait()监视fd_的读事件。
void Channel::enablereading()                    
{
    events_|=EPOLLIN;
    loop_->updatechannel(this);
}

// 把inepoll_成员的值设置为true。
void Channel::setinepoll()                           
{
    inepoll_=true;
}

// 设置revents_成员的值为参数ev。
void Channel::setrevents(uint32_t ev)         
{
    revents_=ev;
}

// 返回inepoll_成员。
bool Channel::inpoll()                                  
{
    return inepoll_;
}

// 返回events_成员。
uint32_t Channel::events()                           
{
    return events_;
}

// 返回revents_成员。
uint32_t Channel::revents()                          
{
    return revents_;
} 

// 事件处理函数，epoll_wait()返回的时候，执行它。
void Channel::handleevent()
{
    if (revents_ & EPOLLRDHUP) // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
    {
        printf("EPOLLRDHUP\n");
        closecallback_();
    }  //  普通数据  带外数据
    else if (revents_ & (EPOLLIN|EPOLLPRI))   // 接收缓冲区中有数据可以读。
    {
        printf("EPOLLIN|EPOLLPRI\n");
        readcallback_();
    }
    else if (revents_ & EPOLLOUT)                  // 有数据需要写，暂时没有代码，以后再说。
    {
        printf("EPOLLOUT\n");
        writecallback_();
    }
    else                                                                   // 其它事件，都视为错误。
    {
        printf("ERROR\n");
        errorcallback_();
    }
}

// 设置fd_读事件的回调函数。
void Channel::setreadcallback(std::function<void()> fn)    
{
    readcallback_ = fn;
}

//设置fd_关闭的回调函数
void Channel::setclosecallback(std::function<void()> fn)
{
    closecallback_ = fn;
}

//设置fd_发生错误的回调函数
void Channel::seterrorcallback(std::function<void()> fn)
{
    errorcallback_ = fn;
}

// 取消读事件      
void Channel::disablereading()
{
    events_ = events_ & ~EPOLLIN;
    loop_->updatechannel(this);
}

//注册写事件
void Channel::enablewriting()
{
    events_ = events_ | EPOLLOUT;
    loop_->updatechannel(this);
}

//取消写事件
void Channel::disablewriting()
{
    events_ = events_ & ~EPOLLOUT;
    loop_->updatechannel(this);
}

//设置写事件的回调函数
void Channel::setwritecallback(std::function<void()> fn)
{
    writecallback_ = fn;
}

//取消全部的事件
void Channel::disableall()
{
    events_ = 0;
    loop_->updatechannel(this);
}

//在从事件循环中删除channel
void Channel::remove()
{
    disableall();  //取消全部的事件
    loop_->removechannel(this); // 从红黑树上删除fd
}