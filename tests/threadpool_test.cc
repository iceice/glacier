#include "glacier/base/threadpool.h"
#include "glacier/base/countdown.h"
#include "glacier/base/current_thread.h"
#include "glacier/base/log.h"

#include <iostream>

using namespace glacier;

void print() { printf("tid=%d\n", glacier::CurrentThread::tid()); }

void printString(const std::string& str) {
  LOG_INFO << str;
  usleep(100 * 1000);
}

void test(int n) {
  LOG_WARN << "Test ThreadPool with max queue size = " << n;
  ThreadPool pool(n);
  LOG_WARN << "Adding";
  for (int i = 0; i < 100; ++i) {
    char buf[32];
    snprintf(buf, sizeof buf, "task %d", i);
    pool.commit(printString, std::string(buf));
  }
  LOG_WARN << "Done";
  glacier::CountDownLatch latch(1);
  pool.commit(std::bind(&glacier::CountDownLatch::countDown, &latch));
  latch.wait();
}

int main(int argc, char const* argv[]) {
  // test(4);
  test(10);
  return 0;
}
