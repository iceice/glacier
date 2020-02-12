#ifndef SRC_EVENT_LOOP_THREAD_
#define SRC_EVENT_LOOP_THREAD_

#include "base/thread.h"
#include "base/mutex.h"
#include "base/condition.h"

class EventLoop;

class EventLoopThread : Uncopyable
{
public:
    typedef std::function<void(EventLoop *)> ThreadInitCallback;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                    const string &name = string());
    
    ~EventLoopThread();
    
    EventLoop *startLoop();

private:
    void threadFunc();

    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
    ThreadInitCallback callback_;
};

#endif // SRC_EVENT_LOOP_THREAD_
