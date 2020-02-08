#ifndef BASE_CONDITION_
#define BASE_CONDITION_

#include "base/mutex.h"

class Condition : Uncopyable
{
public:
    explicit Condition(MutexLock &_mutex) : mutex_(_mutex)
    {
        pthread_cond_init(&pcond_, NULL);
    }

    ~Condition() { pthread_cond_destroy(&pcond_); }

    void wait() { pthread_cond_wait(&pcond_, mutex_.get()); }

    void notify() { pthread_cond_signal(&pcond_); }

    void notifyAll() { pthread_cond_broadcast(&pcond_); }

    // returns true if time out, false otherwise.
    bool waitForSeconds(double seconds);

private:
    MutexLock &mutex_;
    pthread_cond_t pcond_;
};

#endif // BASE_CONDITION_
