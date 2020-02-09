#ifndef BASE_THREAD_POOL_
#define BASE_THREAD_POOL_

#include "base/condition.h"
#include "base/mutex.h"
#include "base/thread.h"
#include "base/types.h"

#include <deque>
#include <vector>
#include <memory>

class ThreadPool : Uncopyable
{
public:
    typedef std::function<void()> Task;

    explicit ThreadPool(const string &nameArg = string("ThreadPool"));
    ~ThreadPool();

    // Must be called before start().
    void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
    void setThreadInitCallback(const Task &cb) { threadInitCallback_ = cb; }

    void start(int numThreads);
    void stop();

    const string &name() const { return name_; }

    size_t queueSize() const;

    void run(Task f);

private:
    bool isFull() const;
    void runInThread();
    Task take();

    mutable MutexLock mutex_;
    Condition notEmpty_;
    Condition notFull_;
    string name_;

    Task threadInitCallback_;                      //线程执行前的回调函数
    std::vector<std::unique_ptr<Thread>> threads_; // 线程数组
    std::deque<Task> queue_;                       // 任务队列
    size_t maxQueueSize_;                          // 线程数的最大值
    bool running_;                                 // 运行标志
};

#endif // BASE_THREAD_POOL_
