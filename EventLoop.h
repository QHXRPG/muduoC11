#pragma once
#include "Epoll.h"

class Channel;
class Epoll;

//  封装事件循环类
class EventLoop
{
private:
    Epoll *ep_;          //每个事件循环只有一个Epoll
public:
    EventLoop();       // 在构造函数中创建Epoll对象ep_
    ~EventLoop();      // 在析构函数中销毁Epoll对象ep_

    void run();       // 运行事件循环
    Epoll *ep();     //返回epoll对象

    // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
    void updatechannel(Channel *ch);  
};