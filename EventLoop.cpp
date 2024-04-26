#include "EventLoop.h"

// 在构造函数中创建Epoll对象ep_
EventLoop::EventLoop()
:ep_(new Epoll)
{
}

// 在析构函数中销毁Epoll对象ep_
EventLoop::~EventLoop()
{
    delete ep_;
}

void EventLoop::run()
{
    while (true)        // 事件循环。
    {
        std::vector<Channel *> Channels = ep_->loop();    // 存放epoll_wait()返回事件。等待监视的fd有事件发生。

        // 遍历epoll返回的数组Channels。
        for (int ii=0; ii<Channels.size(); ii++)
        {
             Channels[ii]->handleevent();  // 处理epoll_wait返回的事件
        }
    }
}

Epoll *EventLoop::ep()
{
    return ep_;
}

// 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
void EventLoop::updatechannel(Channel *ch)
{
    ep_->updatechannel(ch);
}