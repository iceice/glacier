#include "glacier/base/countdown.h"

using namespace glacier;

typedef std::unique_lock<std::mutex> UniqueLock;

CountDownLatch::CountDownLatch(int count) : count_(count) {}

void CountDownLatch::wait() {
  UniqueLock lock(mutex_);
  condition_.wait(lock, [this] { return this->count_ == 0; });
}

void CountDownLatch::countDown() {
  UniqueLock lock(mutex_);
  --count_;
  if (count_ == 0) condition_.notify_all();
}

int CountDownLatch::getCount() {
  UniqueLock lock(mutex_);
  return count_;
}
