#ifndef SRC_EVENT_LOOP_
#define SRC_EVENT_LOOP_

#include "base/mutex.h"
#include "base/condition.h"
#include "base/current_thread.h"
#include "base/timestamp.h"
#include "src/timer_id.h"

#include <functional>
#include <atomic>
#include <memory>
#include <vector>

class Channel;
class Epoll;
class TimerQueue;

//
// Reactor, one loop per thread.
//
// 在这种模型下，程序里的IO线程有且仅有一个event loop，用于处理读写和定时事件。
// EventLoop 代表了程序的主循环，需要让哪个线程干活，就把 Timer或者IO channel
// 注册到哪个线程的loop里即可。
class EventLoop : Uncopyable
{
public:
    typedef std::function<void()> Functor;
    typedef std::function<void()> TimerCallback;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    // poll返回的时间
    Timestamp pollReturnTime() const { return pollReturnTime_; }

    int64_t iteration() const { return iteration_; }

    // 在EventLoop中执行回调函数
    void runInLoop(Functor cb);

    // 将回调函数放入待执行队列
    void queueInLoop(Functor cb);

    size_t queueSize() const;

    // timers

    // 某个时间点执行定时回调
    TimerId runAt(Timestamp time, TimerCallback cb);

    // 某个时间点之后执行定时回调
    TimerId runAfter(double delay, TimerCallback cb);

    // 在每个时间间隔处理某个回调函数
    TimerId runEvery(double interval, TimerCallback cb);

    // 取消某个定时器
    void cancel(TimerId timerId);

    void wakeup();                        // 唤醒事件通知描述符
    void updateChannel(Channel *channel); // 添加某个事件分发器
    void removeChannel(Channel *channel); // 移除某个事件分发器
    bool hasChannel(Channel *channel);

    // 因为有些成员函数只能被IO线程调用，而静态函数getEventLoopOfCurrentThread
    // 的作用是返回EventLoop对象，可能被其他线程调用。所以需要判断当前线程是不是
    // 创建EventLoop的线程。
    void assertInLoopThread()
    {
        if (!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
    bool eventHandling() const { return eventHandling_; }

    // 返回当前线程的EventLoop对象
    static EventLoop *getEventLoopOfCurrentThread();

private:
    void abortNotInLoopThread();
    void handleRead();
    void doPendingFunctors();

    void printActiveChannels() const; // DEBUG

    typedef std::vector<Channel *> ChannelList;

    bool looping_;                // 线程是否执行loop()函数
    std::atomic<bool> quit_;      // 线程是否退出loop()函数
    bool eventHandling_;          // 当前是否在处理事件
    bool callingPendingFunctors_; // 队列中是否有待处理函数
    int64_t iteration_;           // 循环次数
    const pid_t threadId_;        // 线程id

    Timestamp pollReturnTime_;               // poll有事件到来返回的时间
    std::unique_ptr<Epoll> poller_;          // poll函数的具体调用者
    std::unique_ptr<TimerQueue> timerQueue_; // 定时器

    int wakeupFd_;                           // 唤醒fd
    std::unique_ptr<Channel> wakeupChannel_; // 唤醒通道
    ChannelList activeChannels_;             // 活跃的通道列表
    Channel *currentActiveChannel_;          // 当前活跃的通道列表

    mutable MutexLock mutex_;
    std::vector<Functor> pendingFunctors_; // 函数列表
};

#endif // SRC_EVENT_LOOP_
