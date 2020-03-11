#ifndef GLACIER_BASE_CURRENT_THREAD_
#define GLACIER_BASE_CURRENT_THREAD_

#include <stdint.h>
#include <stdio.h>

namespace glacier {
namespace CurrentThread {

extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char* t_threadName;

void cacheTid();

inline int tid() {
  if (t_cachedTid == 0) cacheTid();
  return t_cachedTid;
}

inline const char* tidString() { return t_tidString; }

inline int tidStringLength() { return t_tidStringLength; }

inline const char* name() { return t_threadName; }

bool isMainThread();

void sleepUsec(int64_t usec);

}  // namespace CurrentThread
}  // namespace glacier

#endif  // GLACIER_BASE_CURRENT_THREAD_
