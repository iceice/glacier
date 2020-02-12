#ifndef SRC_EPOLL_
#define SRC_EPOLL_

#include "base/timestamp.h"
#include "src/event_loop.h"

#include <map>
#include <vector>

class Channel;
struct epoll_event;

class Epoll
{
public:
    typedef std::vector<Channel *> ChannelList;

    Epoll(EventLoop *loop);

    ~Epoll();

    Timestamp poll(int timeoutMs, ChannelList *activeChannels);

    void updateChannel(Channel *channel);

    void removeChannel(Channel *channel);

    bool hasChannel(Channel *channel) const;

    void assertInLoopThread() const
    {
        ownerLoop_->assertInLoopThread();
    }

private:
    typedef std::vector<struct epoll_event> EventList;
    typedef std::map<int, Channel *> ChannelMap;

    static const int kInitEventListSize = 16;

    static const char *operationToString(int op);

    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

    void update(int operation, Channel *channel);

    EventLoop *ownerLoop_;

    int epollfd_;

    EventList events_;

    ChannelMap channels_;
};

#endif // SRC_EPOLL_
