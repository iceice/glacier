#ifndef SRC_EPOLL_
#define SRC_EPOLL_

#include "src/channel.h"
#include "src/http_data.h"
#include "src/timer.h"

#include <sys/epoll.h>
#include <unordered_map>
#include <vector>

class Epoll
{
public:
    Epoll();
    
    ~Epoll();
    
    void epoll_add(SP_Channel channel, int timeout);
    void epoll_mod(SP_Channel channel, int timeout);
    void epoll_del(SP_Channel channel);
    
    std::vector<std::shared_ptr<Channel>> poll();
    std::vector<std::shared_ptr<Channel>> getEventsRequest(int events_num);

    void add_timer(SP_Channel channel, int timeout);

    void handleExpired();

    int getEpollFd() { return epollFd_; }

private:
    typedef std::vector<struct epoll_event> EventList;
    static const int MAXFDS = 100000; // 最大处理的文件描述符
    
    int epollFd_;
    EventList events_;
    std::shared_ptr<Channel> fd2chan_[MAXFDS];
    std::shared_ptr<HttpData> fd2http_[MAXFDS];
    TimerManager timerManager_;
};

#endif // SRC_EPOLL_