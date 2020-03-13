#ifndef GLACIER_EPOLL_
#define GLACIER_EPOLL_

#include <sys/epoll.h>
#include <vector>
#include <memory>
#include "glacier/timer.h"

class Channel;
class HttpData;

typedef std::shared_ptr<HttpData> HttpDataPtr;
typedef std::shared_ptr<Channel> ChannelPtr;
typedef std::vector<ChannelPtr> ChannelList;

class Epoll {
 public:
  typedef std::shared_ptr<Epoll> ptr;
  
  Epoll();
  ~Epoll() {}

  void epoll_add(ChannelPtr channel, int timeout);
  void epoll_mod(ChannelPtr channel, int timeout);
  void epoll_del(ChannelPtr channel);

  ChannelList poll();
  ChannelList getActiveChannels(int eventNum);

  int getFd() { return epollfd_; }

  void add_timer(ChannelPtr channel, int timeout);
  void handleExpired();

 private:
  static const int MAXFDS = 100000;

  int epollfd_;
  std::vector<epoll_event> events_;
  ChannelPtr fd2chan_[MAXFDS];
  HttpDataPtr fd2http_[MAXFDS];
  TimerManager timerManager_;
};

#endif  // GLACIER_EPOLL_