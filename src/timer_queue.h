#ifndef SRC_TIMER_QUEUE_
#define SRC_TIMER_QUEUE_

#include "base/timestamp.h"
#include "src/channel.h"

#include <set>
#include <vector>

class EventLoop;
class Timer;
class TimerId;

// 计时器队列
//
// 需要高效地组织目前尚未到期的Timer，能够快速地根据
// 当前时间找到已经到期的Timer，也要能高效地添加和删除
class TimerQueue : Uncopyable
{
public:
    typedef std::function<void()> TimerCallback;
    
    explicit TimerQueue(EventLoop *loop);
    ~TimerQueue();

    TimerId addTimer(TimerCallback cb,
                     Timestamp when,
                     double interval);

    void cancel(TimerId timerId);

private:
    // pair<Timestamp, Timer *>作为key
    // 即使两个Timer的到期时间相同，它们的地址也必不同
    typedef std::pair<Timestamp, Timer *> Entry;
    typedef std::set<Entry> TimerList;
    typedef std::pair<Timer *, int64_t> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;

    void addTimerInLoop(Timer *timer);
    void cancelInLoop(TimerId timerId);

    void handleRead(); // 当timerfd触发时调用
    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry> &expired, Timestamp now);
    bool insert(Timer *timer);

    EventLoop *loop_;
    const int timerfd_;
    Channel timerfdChannel_;

    TimerList timers_;            // Timer队列，按照过期时间排序
    ActiveTimerSet activeTimers_; // 有效的Timer队列

    bool callingExpiredTimers_;
    ActiveTimerSet cancelingTimers_;
};

#endif // SRC_TIMER_QUEUE_