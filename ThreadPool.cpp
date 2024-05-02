#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threadnum, const std::string &threadtype)  // 创建threadnum个线程
:stop_(false),
threadtype_(threadtype)
{
    // 启动threadnum个线程，每个线程将阻塞在条件变量上。
	for (size_t ii = 0; ii < threadnum; ii++)
    {
        // 用ThreadFunction()创建线程。
		threads_.emplace_back([this]
        {
            printf("create %s thread(%d).\n", threadtype_.c_str(), syscall(SYS_gettid));     // 显示线程类型和ID。
            std::cout << "子线程：" << std::this_thread::get_id() << std::endl;

            while (stop_ == false)
            {
                std::function<void()> task;       // 用于存放出队的元素。

                {   /********************锁作用域的开始***************/
                    std::unique_lock<std::mutex> lock(this->mutex_);

                    // 等待生产者的条件变量。如果lambda返回true，条件变量被触发,否则阻塞等待
                    // this指代当前对象的指针，用来访问当前对象的成员变量和成员函数
                    this->condition_.wait(lock, [this] 
                    { 
                        return ((this->stop_==true) || (this->taskqueue_.empty()==false));
                    });

                    // 在线程池停止之前，如果队列中还有任务，执行完再退出。
                    if ((this->stop_==true)&&(this->taskqueue_.empty()==true)) return;

                    // 出队一个任务。
                    task = std::move(this->taskqueue_.front());  // 采用移动语义，避免拷贝操作
                    this->taskqueue_.pop();
                }   /****************锁作用域的结束。锁自动释放****************/

                printf("thread is (%s)%d.\n",threadtype_.c_str(),syscall(SYS_gettid));
                task();  // 执行任务。
            }
        });
    }
}

// 把任务添加到队列中。
void ThreadPool::addtask(std::function<void()> task)
{
    {   /********************锁作用域的开始***********************/
        std::lock_guard<std::mutex> lock(mutex_);
        taskqueue_.push(task);
    }   /****************锁作用域的结束。锁自动释放****************/

    condition_.notify_one();   // 唤醒一个线程。
}

ThreadPool::~ThreadPool()
{
	stop_ = true;

	condition_.notify_all();  // 唤醒全部的线程。

    // 等待全部线程执行完任务后退出。
	for (std::thread &th : threads_) 
        th.join();
}