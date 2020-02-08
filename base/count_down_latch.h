#ifndef BASE_COUNT_DOWN_LATCH_
#define BASE_COUNT_DOWN_LATCH_

#include "base/condition.h"
#include "base/mutex.h"

class CountDownLatch : Uncopyable
{
public:
    explicit CountDownLatch(int count);

    void wait();

    void countDown();

    int getCount() const;

private:
    mutable MutexLock mutex_;
    Condition condition_;
    int count_;
};

#endif // BASE_COUNT_DOWN_LATCH_
