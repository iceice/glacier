#ifndef SRC_TIMER_
#define SRC_TIMER_

#include <unistd.h>
#include <deque>
#include <memory>
#include <queue>
#include "src/http_data.h"
#include "base/mutex.h"
#include "base/uncopyable.h"

class HttpData;

class TimerNode
{
public:
    TimerNode(std::shared_ptr<HttpData> requestData, int timeout);
    ~TimerNode();
    TimerNode(TimerNode &tn);
    void update(int timeout);
    bool isValid();
    void clearReq();
    void setDeleted() { deleted_ = true; }
    bool isDeleted() const { return deleted_; }
    size_t getExpTime() const { return expiredTime_; }

private:
    std::shared_ptr<HttpData> SPHttpData;
    bool deleted_;
    size_t expiredTime_;
};

struct TimerCmp
{
    bool operator()(std::shared_ptr<TimerNode> &a,
                    std::shared_ptr<TimerNode> &b) const
    {
        return a->getExpTime() > b->getExpTime();
    }
};

class TimerManager
{
public:
    TimerManager();
    ~TimerManager();
    void addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout);
    void handleExpiredEvent();

private:
    typedef std::shared_ptr<TimerNode> SPTimerNode;
    std::priority_queue<SPTimerNode, std::deque<SPTimerNode>, TimerCmp>
        timerNodeQueue;
    // MutexLock lock;
};
#endif // SRC_TIMER_
