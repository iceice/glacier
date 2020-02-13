#include "base/thread.h"
#include "base/logging.h"
#include "src/acceptor.h"
#include "src/inet_address.h"
#include "src/socket_ops.h"
#include "src/event_loop.h"
#include "src/server.h"

#include <stdio.h>
#include <unistd.h>


int main(int argc, char const *argv[])
{
    printf("main(): pid = %d\n", getpid());
    InetAddress listenAddr(2020);
    EventLoop mainLoop;
    Server myHTTPServer(&mainLoop, listenAddr, 4);
    myHTTPServer.start();
    mainLoop.loop();
    return 0;
}
