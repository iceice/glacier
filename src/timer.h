#ifndef SRC_TIMER_
#define SRC_TIMER_

#include "base/uncopyable.h"
#include "base/timestamp.h"

#include <atomic>
#include <functional>

typedef std::function<void()> TimerCallback;

class Timer : Uncopyable
{
public:

    Timer(TimerCallback cb, Timestamp when, double interval)
        : callback_(std::move(cb)),
          expiration_(when),
          interval_(interval),
          repeat_(interval > 0.0),
          sequence_(s_numCreated_.fetch_add(1))
    {
    }

    void run() const { callback_(); }

    Timestamp expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return sequence_; }

    void restart(Timestamp now);

    static int64_t numCreated() { return s_numCreated_; }

private:
    const TimerCallback callback_; // 定时器回调函数
    Timestamp expiration_;         // 超时时间
    const double interval_;        // 超时时间间隔，如果是一次性定时器，该值为0
    const bool repeat_;            // 是否重复
    const int64_t sequence_;       // 定时器序号

    static std::atomic_long s_numCreated_;
};

#endif // SRC_TIMER_
