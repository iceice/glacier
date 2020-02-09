#include "base/thread_pool.h"

#include <assert.h>
#include <stdio.h>

ThreadPool::ThreadPool(const string &nameArg)
    : mutex_(),
      notEmpty_(mutex_),
      notFull_(mutex_),
      name_(nameArg),
      maxQueueSize_(0),
      running_(false)
{
}

ThreadPool::~ThreadPool()
{
    if (running_)
    {
        // 如果线程正在处理，我们就要关闭掉正在执行的线程
        stop();
    }
}

void ThreadPool::start(int numThreads)
{
    assert(threads_.empty());     //确认线程数组是空的
    running_ = true;              // 启动标志
    threads_.reserve(numThreads); // 预留空间，即改变vector的capacity
    for (int i = 0; i < numThreads; ++i)
    {
        char id[32]; // 线程id
        snprintf(id, sizeof id, "%d", i + 1);
        
        // bind在绑定类内部成员时，第二个参数必须是类的实例
        threads_.emplace_back(
            new Thread(std::bind(&ThreadPool::runInThread, this), name_ + id));

        //启动每个线程，但是由于线程运行函数是runInThread，所以会阻塞。
        threads_[i]->start();
    }
    if (numThreads == 0 && threadInitCallback_)
    {
        threadInitCallback_();
    }
}

void ThreadPool::stop()
{
    {
        MutexLockGuard lock(mutex_);
        running_ = false;
        notEmpty_.notifyAll(); // 让阻塞在notEmpty contition上的所有线程执行完毕
    }
    for (auto &thr : threads_)
    {
        thr->join(); // 对每个线程调用，pthread_join(),防止资源泄漏
    }
}

size_t ThreadPool::queueSize() const
{
    MutexLockGuard lock(mutex_);
    return queue_.size();
}

void ThreadPool::run(Task task)
{
    if (threads_.empty())
    {
        // 如果线程池为空，说明线程池未分配线程
        task();
    }
    else
    {
        MutexLockGuard lock(mutex_);
        while (isFull())
        {
            // 当任务队列满的时候，循环等待
            notFull_.wait();
        }
        assert(!isFull()); // 再次确认

        queue_.push_back(std::move(task)); // 当任务队列不满，就把该任务加入线程池的任务队列
        notEmpty_.notify();                // 唤醒take()取任务函数，让线程来取任务，取完任务后runInThread会执行任务
    }
}

ThreadPool::Task ThreadPool::take()
{
    // take 函数是每个线程都要执行的，所以需要考虑线程安全
    MutexLockGuard lock(mutex_);
    // always use a while-loop, due to spurious wakeup
    while (queue_.empty() && running_)
    {
        // 如果任务队列为空，并且线程池处于运行态
        // 所有线程通过 notEmpty_ 条件变量等待任务的分配
        notEmpty_.wait();
    }
    Task task;
    if (!queue_.empty())
    {
        task = queue_.front(); // 从任务队列队头中取任务
        queue_.pop_front();
        if (maxQueueSize_ > 0)
        {
            // 取出一个任务之后，如果任务队列长度大于0，唤醒 notfull 未满锁
            notFull_.notify();
        }
    }
    return task;
}

bool ThreadPool::isFull() const
{
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

void ThreadPool::runInThread()
{
    try
    {
        if (threadInitCallback_)
        {
            threadInitCallback_();
        }
        while (running_)
        {
            Task task(take());
            if (task)
            {
                task();
            }
        }
    }
    catch (const std::exception &ex)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    }
    catch (...)
    {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
        throw; // rethrow
    }
}