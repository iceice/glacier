#ifndef SRC_ACCEPTOR_
#define SRC_ACCEPTOR_

#include "src/channel.h"
#include "src/socket.h"

#include <functional>

class EventLoop;
class InetAddress;

class Acceptor : Uncopyable
{
public:
    typedef std::function<void(int sockfd, const InetAddress &)> NewConnectionCallback;

    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();
    
    void setNewConnectionCallback(const NewConnectionCallback &cb)
    {
        newConnectionCallback_ = cb;
    }

    bool listenning() const { return listenning_; }
    void listen();

private:
    void handleRead();

    EventLoop *loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
    int idleFd_;
};

#endif // SRC_ACCEPTOR_
