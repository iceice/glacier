#ifndef GLACIER_EVENTLOOP_
#define GLACIER_EVENTLOOP_

#include <assert.h>
#include <mutex>
#include <vector>
#include "glacier/base/current_thread.h"
#include "glacier/channel.h"
#include "glacier/epoll.h"

/*
 * Reactor, one loop per thread.
 *
 * 在这种模型下，程序里的IO线程有且仅有一个event loop，用于处理读写和定时事件。
 * EventLoop 代表了程序的主循环，需要让哪个线程干活，就把 Timer或者IO channel
 * 注册到哪个线程的loop里即可。
 */
class EventLoop {
 public:
  typedef std::function<void()> Functor;

  EventLoop();
  ~EventLoop();

  void loop();
  void quit();

  // 在EventLoop中执行回调函数
  void runInLoop(Functor &&cb);
  // 将回调函数放入待执行队列
  void queueInLoop(Functor &&cb);

  /*
   * 因为有些成员函数只能被IO线程调用，而静态函数getEventLoopOfCurrentThread
   * 的作用是返回EventLoop对象，可能被其他线程调用。所以需要判断当前线程是不是
   * 创建EventLoop的线程。
   */
  bool isInLoopThread() const {
    return threadId_ == glacier::CurrentThread::tid();
  }
  void assertInLoopThread() { assert(isInLoopThread()); }

  void addToPoller(ChannelPtr channel, int timeout = 0) {
    poller_->epoll_add(channel, timeout);
  }
  void updatePoller(ChannelPtr channel, int timeout = 0) {
    poller_->epoll_mod(channel, timeout);
  }
  void removeFronPoller(ChannelPtr channel) {
    poller_->epoll_del(channel);
  }

 private:
  void wakeup();
  void handleRead();
  void handleConn();
  void doPendingFunctors();
  
  bool looping_;                 // 线程是否执行loop()函数
  bool quit_;                    // 线程是否退出loop()函数
  bool eventHandling_;           // 当前是否在处理事件
  bool callingPendingFunctors_;  // 队列中是否有待处理函数
  const pid_t threadId_;         // 线程id
  int wakeupFd_;                 // 唤醒fd
  ChannelPtr pwakeupChannel_;  // 唤醒通道
  Epoll::ptr poller_;            // poll函数的具体调用者

  std::mutex mutex_;
  std::vector<Functor> pendingFunctors_;
};

#endif  // GLACIER_EVENTLOOP_