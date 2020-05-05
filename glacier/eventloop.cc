#include "glacier/eventloop.h"
#include <sys/eventfd.h>
#include "glacier/base/current_thread.h"
#include "glacier/base/logging.h"
#include "glacier/utils.h"

#include <memory>

using namespace std;

__thread EventLoop *t_loopInThisThread = 0;

int createEventfd() {
  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG_ERROR << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      threadId_(glacier::CurrentThread::tid()),
      wakeupFd_(createEventfd()),
      pwakeupChannel_(new Channel(this, wakeupFd_)),
      poller_(new Epoll()) {
  if (t_loopInThisThread) {
    LOG_ERROR << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }
  pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
  pwakeupChannel_->setReadCallback(bind(&EventLoop::handleRead, this));
  pwakeupChannel_->setConnCallback(bind(&EventLoop::handleConn, this));
  poller_->epoll_add(pwakeupChannel_, 0);
}

EventLoop::~EventLoop() {
  ::close(wakeupFd_);
  t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;
  ChannelList activeChannels;
  while (!quit_) {
    activeChannels.clear();
    activeChannels = poller_->poll();
    eventHandling_ = true;
    for (auto &it : activeChannels) it->handleEvents();
    eventHandling_ = false;
    doPendingFunctors();
    poller_->handleExpired();
  }
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  if (!isInLoopThread()) wakeup();
}

void EventLoop::runInLoop(Functor &&cb) {
  if (isInLoopThread()) {
    cb();  // 如果在IO线程，则直接执行
  } else {
    queueInLoop(move(cb));  // 否则，将任务放入任务队列
  }
}

void EventLoop::queueInLoop(Functor &&cb) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pendingFunctors_.emplace_back(move(cb));
  }
  if (!isInLoopThread() || callingPendingFunctors_) wakeup();
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = ::write(wakeupFd_, reinterpret_cast<char *>(&one), sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = ::read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
  pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
}

void EventLoop::handleConn() { updatePoller(pwakeupChannel_, 0); }

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    functors.swap(pendingFunctors_);
  }

  for (size_t i = 0; i < functors.size(); ++i) functors[i]();
  callingPendingFunctors_ = false;
}