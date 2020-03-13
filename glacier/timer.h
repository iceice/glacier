#ifndef GLACIER_TIMER_
#define GLACIER_TIMER_

#include <memory>
#include <queue>
#include <vector>
#include "time.h"

class HttpData;

typedef std::shared_ptr<HttpData> HttpDataPtr;

class Timer {
 public:
  typedef std::shared_ptr<Timer> ptr;

  Timer(HttpDataPtr req, int timeout);
  Timer(const Timer &other);
  ~Timer();

  // 更新过期时间
  void update(int timeout);
  time_t getExpTime() const { return expiredtime_; }

  bool isValid();

  void reset();

  void setDeleted() { deleted_ = true; }
  bool isDeleted() const { return deleted_; }

 private:
  bool deleted_;          // 是否被删除
  time_t expiredtime_;    // 过期时间
  HttpDataPtr httpdata_;  // 每个计时器对应一个httpconnection
};

struct TimerCmp {
  bool operator()(Timer::ptr &a, Timer::ptr &b) const {
    return a->getExpTime() > b->getExpTime();
  }
};

class TimerManager {
 public:
  TimerManager() {}
  ~TimerManager() {}
  void addTimer(HttpDataPtr req, int timeout);
  void handleExpiredEvent();

 private:
  std::priority_queue<Timer::ptr, std::vector<Timer::ptr>, TimerCmp> heap_;
};

#endif  // GLACIER_TIMER_