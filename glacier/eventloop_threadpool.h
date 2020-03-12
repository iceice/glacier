#ifndef GLACIER_EVENTLOOP_THREADPOOL_
#define GLACIER_EVENTLOOP_THREADPOOL_

#include "glacier/base/uncopyable.h"
#include "glacier/eventloop.h"
#include "glacier/eventloop_thread.h"

class EventLoopThreadPool : Uncopyable {
 public:
  typedef std::shared_ptr<EventLoopThreadPool> ptr;

  EventLoopThreadPool(EventLoop* baseloop, int numThreads);
  ~EventLoopThreadPool() {}

  void start();

  EventLoop* getNextLoop();

 private:
  EventLoop* baseloop_;
  bool started_;
  int numThreads_;
  int next_;
  std::vector<std::shared_ptr<EventLoopThread> > threads_;
  std::vector<EventLoop*> loops_;
};

#endif  // GLACIER_EVENTLOOP_THREADPOOL_