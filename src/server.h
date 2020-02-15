#ifndef SRC_SERVER_
#define SRC_SERVER_

#include <memory>
#include "src/channel.h"
#include "src/event_loop.h"
#include "src/event_loop_thread_pool.h"

class Server
{
public:
    Server(EventLoop *loop, int threadNum, int port);
    ~Server() {}
    EventLoop *getLoop() const { return loop_; }
    void start();
    void handNewConn();
    void handThisConn() { loop_->updatePoller(acceptChannel_); }

private:
    EventLoop *loop_;
    int listenFd_;
    int threadNum_;
    std::unique_ptr<EventLoopThreadPool> eventLoopThreadPool_;
    bool started_;
    std::shared_ptr<Channel> acceptChannel_;
    int port_;
    static const int MAXFDS = 100000;
};
#endif // SRC_SERVER_
