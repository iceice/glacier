#include "glacier/epoll.h"
#include <assert.h>
#include "glacier/base/logging.h"
#include "glacier/channel.h"

using namespace std;

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;

Epoll::Epoll() : epollfd_(epoll_create1(EPOLL_CLOEXEC)), events_(EVENTSNUM) {
  assert(epollfd_ > 0);
}

void Epoll::epoll_add(ChannelPtr channel, int timeout) {
  int fd = channel->getFd();
  if (timeout > 0) {
    add_timer(channel, timeout);
    fd2http_[fd] = channel->getHolder();
  }
  struct epoll_event event;
  event.data.fd = fd;
  event.events = channel->getEvents();

  channel->EqualAndUpdateLastEvents();

  fd2chan_[fd] = channel;
  if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &event) < 0) {
    LOG_ERROR << "epoll add error";
    fd2chan_[fd].reset();
  }
}

void Epoll::epoll_mod(ChannelPtr channel, int timeout) {
  if (timeout > 0) add_timer(channel, timeout);
  int fd = channel->getFd();
  if (channel->EqualAndUpdateLastEvents() == false) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = channel->getEvents();
    if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &event) < 0) {
      LOG_ERROR << "epoll mod error";
      fd2chan_[fd].reset();
    }
  }
}

void Epoll::epoll_del(ChannelPtr channel) {
  int fd = channel->getFd();
  struct epoll_event event;
  event.data.fd = fd;
  event.events = channel->getLastEvents();
  if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &event) < 0) {
    LOG_ERROR << "epoll del error";
  }
  fd2chan_[fd].reset();
  fd2http_[fd].reset();
}

ChannelList Epoll::poll() {
  while (1) {
    int cnt = epoll_wait(epollfd_, &*events_.begin(),
                         static_cast<int>(events_.size()), EPOLLWAIT_TIME);
    if (cnt < 0) {
      LOG_ERROR << "epoll wait error";
    }
    ChannelList res = getActiveChannels(cnt);
    if (res.size()) return res;
  }
}

ChannelList Epoll::getActiveChannels(int eventNum) {
  ChannelList res;
  for (int i = 0; i < eventNum; ++i) {
    // 遍历获取有事件产生的文件描述符
    int fd = events_[i].data.fd;
    ChannelPtr active = fd2chan_[fd];
    if (active) {
      active->setRevents(events_[i].events);
      active->setEvents(0);
      res.emplace_back(active);
    }
  }
  return res;
}

void Epoll::add_timer(ChannelPtr channel, int timeout) {
  HttpDataPtr tmp = channel->getHolder();
  if (tmp) {
    timerManager_.addTimer(tmp, timeout);
  } else {
    LOG_ERROR << "epoll add timer error";
  }
}

void Epoll::handleExpired() { timerManager_.handleExpiredEvent(); }