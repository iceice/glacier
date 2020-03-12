#ifndef GLACIER_EVENTLOOP_THREAD_
#define GLACIER_EVENTLOOP_THREAD_

#include <condition_variable>
#include <mutex>
#include <thread>
#include "glacier/base/uncopyable.h"
#include "glacier/eventloop.h"

class EventLoopThread : Uncopyable {
 public:
  EventLoopThread();
  ~EventLoopThread();
  EventLoop* startLoop();

 private:
  void threadFunc();

  EventLoop* loop_;
  bool stoped_;
  std::thread thread_;
  std::mutex mutex_;
  std::condition_variable condition_;
};

#endif  // GLACIER_EVENTLOOP_THREAD_