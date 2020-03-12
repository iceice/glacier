#include "glacier/eventloop_thread.h"
#include <functional>

EventLoopThread::EventLoopThread() : loop_(nullptr), stoped_(false) {}

EventLoopThread::~EventLoopThread() {
  stoped_ = true;
  if (loop_) {
    loop_->quit();
    thread_.join();
  }
}

EventLoop* EventLoopThread::startLoop() {
  thread_ = std::thread(std::bind(&EventLoopThread::threadFunc, this));
  {
    std::unique_lock<std::mutex> lock(mutex_);
    // 一直等到threadFun在Thread里真正跑起来
    while (loop_ == NULL) condition_.wait(lock);
  }
  return loop_;
}

void EventLoopThread::threadFunc() {
  EventLoop loop;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = &loop;
    condition_.notify_all();
  }
  loop.loop();
  loop_ = NULL;
}