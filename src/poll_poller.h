#ifndef SRC_POLL_
#define SRC_POLL_

#include "src/poller.h"

#include <vector>

struct pollfd;

class PollPoller : public Poller
{
public:
    PollPoller(EventLoop *loop);
    ~PollPoller() override;
    
    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;

private:
    void fillActiveChannels(int numEvents,
                            ChannelList *activeChannels) const;

    typedef std::vector<struct pollfd> PollFdList;
    PollFdList pollfds_;
};

#endif // SRC_POLL_
