#pragma once
#include "functional"
#include "Epoll.h"
#include <memory>
#include <sys/syscall.h>
#include <unistd.h>
#include <queue>
#include <mutex>
#include <sys/eventfd.h>
#include <sys/timerfd.h>  // 定时器所要包含的头文件
#include <map>
#include "Connection.h"
#include <atomic>

class Channel;
class Epoll;
class Connection;
using spConnection = std::shared_ptr<Connection>;

// 事件循环类。
class EventLoop
{
private:
    //闹钟时间间隔，单位：秒
    int timetvl_;

    //Connection对象超时的时间，单位：秒
    int timeout_;
    
    // 每个事件循环只有一个Epoll。
    std::unique_ptr<Epoll> ep_;                       

    std::function<void(EventLoop*)> epolltimeoutcallback_;

    //事件循环所在线程的id
    pid_t threadid_;

    //事件循环被eventfd唤醒后执行的任务队列
    std::queue<std::function<void()>> taskqueue_;

    //任务队列同步的互斥锁
    std::mutex mutex_;

    //用于唤醒事件循环线程的eventfd
    int wakeupfd_;

    // eventfd的Channel
    std::unique_ptr<Channel> wakechannel_;

    // 定时器的fd
    int timerfd_;

    //定时器的Channel
    std::unique_ptr<Channel> timerchannel_;

    // true-主事件循环   false-从事件循环
    bool mainloop_;

    //存放运行在该事件上的全部Connection对象
    std::map<int, spConnection> conns_;

    //删除TcpServe中超时的Connection对象， 将被设置为TcpServe::removeconn()
    std::function<void(int)> timercallback_;

    // 保护conns_的互斥锁
    std::mutex mmutex_;

    // 初始值：false，当值为true时，表示退出事件循环
    std::atomic_bool stop_;


public:
    EventLoop(bool mainloop, int timetvl=30, int timeout=80);                   // 在构造函数中创建Epoll对象ep_。
    ~EventLoop();                // 在析构函数中销毁ep_。

    // 运行事件循环。
    void run();      

    //停止事件循环
    void stop();

    // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
    void updatechannel(Channel *ch);  

    void setepolltimeoutcallback(std::function<void(EventLoop*)> fn);

    //从红黑树上删除Channel
    void removechannel(Channel *ch);

    //判断当前线程是否为事件循环线程
    bool isinloopthread();

    // 把任务添加到队列中
    void queueinloop(std::function<void()> fn);

    //用eventfd唤醒事件循环
    void wakeup();

    //事件循环被eventfd唤醒后执行的函数
    void handlewakeup();

    // 闹钟响时执行的函数
    void handeltimer();

    // 把Connection对象保存在conns_中
    void newconnection(spConnection conn);

    // 将被设置为TcpServe::removeconn()
    void settimercallback(std::function<void(int)> fn);
};