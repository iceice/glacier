#include "src/poller.h"

#include "src/channel.h"
#include "src/poll_poller.h"

bool Poller::hasChannel(Channel *channel) const
{
    assertInLoopThread();
    ChannelMap::const_iterator it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}

Poller *Poller::newDefaultPoller(EventLoop *loop)
{
    // if (::getenv("MUDUO_USE_POLL"))
    // {
    //     return new PollPoller(loop);
    // }
    // else
    // {
    //     return new EPollPoller(loop);
    // }
    return new PollPoller(loop);
}