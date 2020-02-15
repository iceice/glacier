#ifndef SRC_EVENT_LOOP_THREAD_
#define SRC_EVENT_LOOP_THREAD_

#include "src/event_loop.h"
#include "base/condition.h"
#include "base/mutex.h"
#include "base/thread.h"
#include "base/uncopyable.h"

class EventLoopThread : Uncopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop *startLoop();

private:
    void threadFunc();
    
    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
};
#endif // SRC_EVENT_LOOP_THREAD_
