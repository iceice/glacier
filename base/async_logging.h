#ifndef BASE_ASYNC_LOGGING_
#define BASE_ASYNC_LOGGING_

// #include "base/blocking_queue.h"
// #include "base/bounded_blocking_queue.h"
#include "base/count_down_latch.h"
#include "base/mutex.h"
#include "base/thread.h"
#include "base/logstream.h"

#include <atomic>
#include <vector>
#include <memory>

class AsyncLogging : Uncopyable
{
public:
    AsyncLogging(const string &basename,
                 off_t rollSize,
                 int flushInterval = 3);

    ~AsyncLogging()
    {
        if (running_)
        {
            stop();
        }
    }

    void append(const char *logline, int len);

    void start()
    {
        running_ = true;
        thread_.start();
        // 线程启动后，执行threadFunc会调用latch_.countDown()
        latch_.wait();
    }

    void stop()
    {
        running_ = false;
        cond_.notify();
        thread_.join();
    }

private:
    void threadFunc();

    typedef FixedBuffer<kLargeBuffer> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;

    const int flushInterval_;   // 日志写入的间隔时间
    std::atomic<bool> running_; // 日志运行标志，原子量
    const string basename_;     // 日志文件的basename
    const off_t rollSize_;      // 日志文件的大小超过rollSize_就生成一个新文件

    Thread thread_;           // 后端日志线程，收集日志消息
    CountDownLatch latch_;    // 倒计时器
    MutexLock mutex_;         // 互斥量
    Condition cond_;          // 条件变量
    BufferPtr currentBuffer_; // 当前buffer
    BufferPtr nextBuffer_;    // 下一个buffer
    BufferVector buffers_;    // 存储需要写入磁盘的buffer数组
};

#endif // BASE_ASYNC_LOGGING_
