#ifndef SRC_EVENT_LOOP_THREAD_POOL_
#define SRC_EVENT_LOOP_THREAD_POOL_

#include <memory>
#include <vector>
#include "src/event_loop_thread.h"
#include "base/logging.h"
#include "base/uncopyable.h"

class EventLoopThreadPool : Uncopyable
{
public:
    EventLoopThreadPool(EventLoop *baseLoop, int numThreads);

    ~EventLoopThreadPool() { LOG_INFO << "~EventLoopThreadPool()"; }
    void start();

    EventLoop *getNextLoop();

private:
    EventLoop *baseLoop_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::shared_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};
#endif // SRC_EVENT_LOOP_THREAD_POOL_
