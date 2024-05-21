#include "EventLoop.h"

//创建定时器
int createtimerfd(int sec=30)
{
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);  // 创建timerfd
    struct itimerspec timeout;  // 定时时间的数据结构
    memset(&timeout, 0, sizeof(itimerspec));
    timeout.it_value.tv_sec = sec;  // 定时时间
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(tfd, 0, &timeout, 0);
    return tfd;
}

// 在构造函数中创建Epoll对象ep_。
EventLoop::EventLoop(bool mainloop, int timetvl, int timeout)
:ep_(new Epoll),
mainloop_(mainloop),
timetvl_(timetvl),
timeout_(timeout),
wakeupfd_(eventfd(0, EFD_NONBLOCK)),
wakechannel_(new Channel(this, wakeupfd_)),
timerfd_(createtimerfd(timeout_)),
timerchannel_(new Channel(this, timerfd_)),
stop_(false)
{
    // 事件循环被唤醒，就会调用 EventLoop::handlewakeup    设置读事件
    wakechannel_->setreadcallback(std::bind(&EventLoop::handlewakeup, this));
    wakechannel_->enablereading();

    // 闹钟响时的回调函数，设置读事件
    timerchannel_->setreadcallback(std::bind(&EventLoop::handeltimer, this));
    timerchannel_->enablereading();
}

// 在析构函数中销毁ep_。
EventLoop::~EventLoop()
{
    
}

// 运行事件循环。也就是IO线程
void EventLoop::run()                      
{
    // 获取事件循环所在线程的id
    threadid_ = syscall(SYS_gettid);
    while (stop_ == false)        // 事件循环。
    {
        // 等待监视的fd有事件发生。
       std::vector<Channel *> channels = ep_->loop(10*1000);         

        // 如果channels为空，表示超时，回调TCPServer::epolltimeout()
        if(channels.size() == 0)
        {
            epolltimeoutcallback_(this);
        }
        else
            for (auto &ch:channels)
            {
                // 处理epoll_wait()返回的事件。
                ch->handleevent();
            }
    }
}

//停止事件循环
void EventLoop::stop()
{
    // 事件循环阻塞在 ep_->loop() 中，即使stop_设置为true， 事件循环也不会立即退出
    // 只有当闹钟响了，或者epoll_wait() 超时了，才会返回
    // 所以当设置了stop_后，还需要立即唤醒事件循环， 让epoll_wait() 返回
    stop_ = true;  
    wakeup(); //唤醒事件循环
}

// 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
void EventLoop::updatechannel(Channel *ch)                        
{
    ep_->updatechannel(ch);
}


void EventLoop::setepolltimeoutcallback(std::function<void(EventLoop*)> fn)
{
    epolltimeoutcallback_ = fn;
}

//从红黑树上删除channel
void EventLoop::removechannel(Channel *ch)
{
    ep_->removechannel(ch);
}

//判断当前线程是否为事件循环线程
bool EventLoop::isinloopthread()
{
    // 将之前获取的id和现在获取的id比较
    return threadid_ == syscall(SYS_gettid);
}

// 把任务添加到队列中
void EventLoop::queueinloop(std::function<void()> fn)
{
    {
        //使用std::lock_guard可以简化互斥量的加锁和解锁操作，避免忘记解锁互斥量而导致死锁的情况
        std::lock_guard<std::mutex> gd(mutex_);  // 给任务队列加锁
        taskqueue_.push(fn); // 任务入队
    }

    //唤醒事件循环
    wakeup();
}

//用eventfd唤醒事件循环
void EventLoop::wakeup()
{
    uint64_t val = 1;
    write(wakeupfd_, &val, sizeof(val));
}

//事件循环被eventfd唤醒后执行的函数
void EventLoop::handlewakeup()
{
    uint64_t val;

    // 一定要读取出来，如果不读取，eventfd的读事件会一直触发
    read(wakeupfd_, &val, sizeof(val));

    std::function<void()> fn;

    // 给任务队列加锁
    std::lock_guard<std::mutex> gd(mutex_);

    //执行任务队列中的全部任务
    while(taskqueue_.size() > 0)
    {
        // 出队一个元素给fn，然后执行任务
        fn = std::move(taskqueue_.front());
        taskqueue_.pop();
        fn();
    }
}

// 闹钟响时执行的函数
void EventLoop::handeltimer()
{
    // 重新计时
    struct itimerspec timeout;  // 定时时间的数据结构
    memset(&timeout, 0, sizeof(itimerspec));
    timeout.it_value.tv_sec = timetvl_;  // 定时时间
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(timerfd_, 0, &timeout, 0);

    if(mainloop_)  // 主事件循环没有Connection对象
    {
    }
    else
    {
        time_t now = time(0); //获取当前时间
        for(auto aa : conns_)
        {
            if(aa.second->timeout(now, timeout_))
            {
                {
                    std::lock_guard<std::mutex> gd(mmutex_);
                    // 如果超时，则删除TcpServe 和 EventLoop 的 map容器中的conn
                    conns_.erase(aa.first); 
                }
                timercallback_(aa.first);  // 从TcpServe的map中删除超时的conn
            }
        }
        printf("\n");
    }
}

// 把Connection对象保存在conns_中
void EventLoop::newconnection(spConnection conn)
{
    
    std::lock_guard<std::mutex> gd(mmutex_);
    // key:当前连接的fd， value：当前连接
    conns_[conn->fd()] = conn; 
    

}

// 将被设置为TcpServe::removeconn()
void EventLoop::settimercallback(std::function<void(int)> fn)
{
    timercallback_ = fn;
}