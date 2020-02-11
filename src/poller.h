#ifndef SRC_POLLER_
#define SRC_POLLER_

#include "base/timestamp.h"
#include "src/event_loop.h"

#include <map>
#include <vector>

class Channel;

//
// 抽象基类
//
// 对IO multiplexing 的封装
class Poller : Uncopyable
{
public:
    typedef std::vector<Channel *> ChannelList;

    Poller(EventLoop *loop) : ownerLoop_(loop) {}
    virtual ~Poller() {}

    // Polls the I/O events.
    // Must be called in the loop thread.
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;

    // Changes the interested I/O events.
    // Must be called in the loop thread.
    virtual void updateChannel(Channel *channel) = 0;

    // Remove the channel, when it destructs.
    // Must be called in the loop thread.
    virtual void removeChannel(Channel *channel) = 0;

    virtual bool hasChannel(Channel *channel) const;

    static Poller *newDefaultPoller(EventLoop *loop);

    void assertInLoopThread() const
    {
        ownerLoop_->assertInLoopThread();
    }

protected:
    typedef std::map<int, Channel *> ChannelMap;
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_;
};

#endif // SRC_POLLER_
