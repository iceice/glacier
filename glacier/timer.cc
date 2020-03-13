#include "glacier/timer.h"
#include <sys/time.h>
#include <unistd.h>
#include "glacier/httpdata.h"

Timer::Timer(HttpDataPtr req, int timeout)
    : deleted_(false), httpdata_(req) {
  struct timeval now;
  gettimeofday(&now, NULL);
  expiredtime_ =
      (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

Timer::Timer(const Timer &other)
    : deleted_(other.deleted_),
      expiredtime_(other.expiredtime_),
      httpdata_(other.httpdata_) {}

Timer::~Timer() {
  // 关闭连接
  if (httpdata_) httpdata_->handleClose();
}

void Timer::update(int timeout) {
  struct timeval now;
  gettimeofday(&now, NULL);
  expiredtime_ =
      (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

bool Timer::isValid() {
  struct timeval now;
  gettimeofday(&now, NULL);
  time_t temp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
  if (temp < expiredtime_) {
    return true;
  } else {
    this->setDeleted();
    return false;
  }
}

void Timer::reset() {
  httpdata_.reset();
  this->setDeleted();
}

void TimerManager::addTimer(HttpDataPtr req, int timeout) {
  Timer::ptr t(new Timer(req, timeout));
  heap_.push(t);
  // 将HttpConnection与timer做一个link
  req->linkTimer(t);
}

void TimerManager::handleExpiredEvent() {
  while (heap_.size()) {
    Timer::ptr now = heap_.top();
    if (now->isDeleted() || !now->isValid()) {
      heap_.pop();
    } else {
      break;
    }
  }
}