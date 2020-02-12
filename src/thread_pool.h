#ifndef SRC_THREAD_POOL_
#define SRC_THREAD_POOL_

#include "base/uncopyable.h"

#include <functional>
#include <memory>
#include <vector>
#include <string>

using std::string;

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : Uncopyable
{
public:
    typedef std::function<void(EventLoop *)> ThreadInitCallback;

    EventLoopThreadPool(EventLoop *baseLoop, const string &nameArg);
    ~EventLoopThreadPool();
    
    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    EventLoop *getNextLoop();

    EventLoop *getLoopForHash(size_t hashCode);

    std::vector<EventLoop *> getAllLoops();

    bool started() const
    {
        return started_;
    }

    const string &name() const
    {
        return name_;
    }

private:
    EventLoop *baseLoop_;
    string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};

#endif // SRC_THREAD_POOL_
