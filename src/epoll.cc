#include "src/epoll.h"
#include "src/utils.h"
#include "base/logging.h"

#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;

typedef shared_ptr<Channel> SP_Channel;

Epoll::Epoll() : epollFd_(epoll_create1(EPOLL_CLOEXEC)), events_(EVENTSNUM)
{
    assert(epollFd_ > 0);
}

Epoll::~Epoll()
{
    ::close(epollFd_);
}

// 注册新描述符
void Epoll::epoll_add(SP_Channel channel, int timeout)
{
    int fd = channel->getFd();
    if (timeout > 0)
    {
        add_timer(channel, timeout);
        fd2http_[fd] = channel->getHolder();
    }
    struct epoll_event event;
    event.data.fd = fd;
    event.events = channel->getEvents();

    channel->EqualAndUpdateLastEvents();

    fd2chan_[fd] = channel;
    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        perror("epoll_add error");
        LOG_ERROR << "epoll add error";
        fd2chan_[fd].reset();
    }
}

// 修改描述符状态
void Epoll::epoll_mod(SP_Channel channel, int timeout)
{
    if (timeout > 0)
        add_timer(channel, timeout);
    int fd = channel->getFd();
    if (!channel->EqualAndUpdateLastEvents())
    {
        struct epoll_event event;
        event.data.fd = fd;
        event.events = channel->getEvents();
        if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) < 0)
        {
            perror("epoll_mod error");
            LOG_ERROR << "epoll mod error";
            fd2chan_[fd].reset();
        }
    }
}

// 从epoll中删除描述符
void Epoll::epoll_del(SP_Channel channel)
{
    int fd = channel->getFd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = channel->getLastEvents();
    if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event) < 0)
    {
        perror("epoll_del error");
        LOG_ERROR << "epoll del error";
    }
    fd2chan_[fd].reset();
    fd2http_[fd].reset();
}

// 返回活跃事件数
std::vector<SP_Channel> Epoll::poll()
{
    while (true)
    {
        int event_count = epoll_wait(epollFd_, 
                                     &*events_.begin(), 
                                     static_cast<int>(events_.size()), 
                                     EPOLLWAIT_TIME);
        if (event_count < 0)
        {
            perror("epoll wait error");
            LOG_SYSERR << "epoll wait error";
        }

        std::vector<SP_Channel> req_data = getEventsRequest(event_count);
        if (req_data.size() > 0)
            return req_data;
    }
}

void Epoll::handleExpired() { timerManager_.handleExpiredEvent(); }

// 分发处理函数
std::vector<SP_Channel> Epoll::getEventsRequest(int events_num)
{
    std::vector<SP_Channel> req_data;
    for (int i = 0; i < events_num; ++i)
    {
        // 获取有事件产生的描述符
        int fd = events_[i].data.fd;
        SP_Channel cur_req = fd2chan_[fd];

        if (cur_req)
        {
            cur_req->setRevents(events_[i].events);
            cur_req->setEvents(0);
            req_data.push_back(cur_req);
        }
        else
        {
            LOG_ERROR << "SP cur_req is invalid";
        }
    }
    return req_data;
}

void Epoll::add_timer(SP_Channel channel, int timeout)
{
    shared_ptr<HttpData> t = channel->getHolder();
    if (t)
        timerManager_.addTimer(t, timeout);
    else
        LOG_ERROR << "timer add fail";
}