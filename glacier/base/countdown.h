#ifndef GLACIER_BASE_COUNTDOWN_
#define GLACIER_BASE_COUNTDOWN_

#include <condition_variable>
#include <mutex>
#include "glacier/base/uncopyable.h"

namespace glacier {

class CountDownLatch : Uncopyable {
 public:
  explicit CountDownLatch(int count);

  void wait();

  void countDown();

  int getCount();

 private:
  std::mutex mutex_;
  std::condition_variable condition_;
  int count_;
};

}  // namespace glacier

#endif  // GLACIER_BASE_COUNTDOWN_