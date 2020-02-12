#include "base/thread.h"
#include "base/logging.h"
#include "src/acceptor.h"
#include "src/inet_address.h"
#include "src/socket_ops.h"
#include "src/event_loop.h"

#include <stdio.h>
#include <unistd.h>

void newConnection(int sockfd, const InetAddress& peerAddr)
{
    printf("newConnection(): accepted a new connection from %s \n",
            peerAddr.toIpPort().c_str());
    ssize_t t = ::write(sockfd, "How are you?\n", 13);
    if (t < 0) return;
    sockets::close(sockfd);
}

int main(int argc, char const *argv[])
{
    printf("main(): pid = %d\n", getpid());
    InetAddress listenAddr(2020);
    EventLoop loop;
    Acceptor acceptor(&loop, listenAddr, false);
    acceptor.setNewConnectionCallback(newConnection);
    acceptor.listen();

    loop.loop();
    return 0;
}
