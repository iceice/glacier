#include "src/event_loop_thread.h"
#include "src/event_loop.h"
#include "base/thread.h"
#include "base/count_down_latch.h"

#include <stdio.h>
#include <unistd.h>

void print(EventLoop *p = NULL)
{
    printf("print: pid = %d, tid = %d, loop = %p\n",
           getpid(), CurrentThread::tid(), p);
}

void quit(EventLoop *p)
{
    print(p);
    p->quit();
}

int main(int argc, char const *argv[])
{
    print();

    {
        EventLoopThread thr1; // never start
    }

    {
        // dtor calls quit()
        EventLoopThread thr2;
        EventLoop *loop = thr2.startLoop();
        loop->runInLoop(std::bind(print, loop));
        CurrentThread::sleepUsec(500 * 1000);
    }

    {
        // quit() before dtor
        EventLoopThread thr3;
        EventLoop *loop = thr3.startLoop();
        loop->runInLoop(std::bind(quit, loop));
        CurrentThread::sleepUsec(500 * 1000);
    }
    return 0;
}
