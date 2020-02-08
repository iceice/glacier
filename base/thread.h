#ifndef BASE_THREAD_
#define BASE_THREAD_

#include "base/count_down_latch.h"
#include "base/uncopyable.h"
#include "base/types.h"

#include <functional>
#include <atomic>

class Thread : private Uncopyable
{
public:
    typedef std::function<void()> ThreadFunc;

    explicit Thread(ThreadFunc, const string &name = string());
    ~Thread();
    
    void start();
    int join(); // return pthread_join()

    bool started() const { return started_; }

private:
    void setDefaultName();

    bool started_;
    bool joined_;
    pthread_t pthreadId_;
    pid_t tid_;
    ThreadFunc func_;
    string name_;
    CountDownLatch latch_;

    static std::atomic_int numCreated_;
};

#endif // BASE_THREAD_
