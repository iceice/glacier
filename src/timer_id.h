#ifndef SRC_TIMER_ID_
#define SRC_TIMER_ID_

#include "base/copyable.h"

class Timer;

// 记录定时器的id，便于快速取消定时器
class TimerId : public Copyable
{
public:
    TimerId()
        : timer_(NULL),
          sequence_(0)
    {
    }

    TimerId(Timer *timer, int64_t seq)
        : timer_(timer),
          sequence_(seq)
    {
    }

    // default copy-ctor, dtor and assignment are okay

    friend class TimerQueue;

private:
    Timer *timer_;
    int64_t sequence_;
};

#endif // SRC_TIMER_ID_
