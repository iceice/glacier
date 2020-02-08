#include "base/condition.h"

#include <errno.h>

// returns true if time out, false otherwise.
bool Condition::waitForSeconds(double seconds)
{
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);
    abstime.tv_sec += static_cast<time_t>(seconds);
    return ETIMEDOUT == pthread_cond_timedwait(&pcond_, mutex_.get(), &abstime);
}