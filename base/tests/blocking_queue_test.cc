#include "base/blocking_queue.h"
#include "base/count_down_latch.h"
#include "base/thread.h"

#include <memory>
#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>

class Test
{
public:
    Test(int numThreads) : latch_(numThreads)
    {
        for (int i = 0; i < numThreads; ++i)
        {
            char name[32];
            snprintf(name, sizeof name, "work thread %d", i);
            threads_.emplace_back(new Thread(
                std::bind(&Test::threadFunc, this), string(name)));
        }
        for (auto &thr : threads_)
        {
            thr->start();
        }
    }

    void run(int times)
    {
        printf("waiting for count down latch\n");
        latch_.wait();
        printf("all threads started\n");
        for (int i = 0; i < times; ++i)
        {
            char buf[32];
            snprintf(buf, sizeof buf, "hello %d", i);
            queue_.put(buf);
            printf("tid=%d, put data = %s, size = %zd\n", CurrentThread::tid(), buf, queue_.size());
        }
    }

    void joinAll()
    {
        for (size_t i = 0; i < threads_.size(); ++i)
            queue_.put("stop");

        for (auto &thr : threads_)
            thr->join();
    }

private:
    void threadFunc()
    {
        printf("tid=%d, %s started\n", CurrentThread::tid(), CurrentThread::name());

        latch_.countDown();
        bool running = true;
        while (running)
        {
            std::string d(queue_.take());
            printf("tid=%d, get data = %s, size = %zd\n", CurrentThread::tid(), d.c_str(), queue_.size());
            running = (d != "stop");
        }

        printf("tid=%d, %s stopped\n", CurrentThread::tid(), CurrentThread::name());
    }

    BlockingQueue<std::string> queue_;
    CountDownLatch latch_;
    std::vector<std::unique_ptr<Thread>> threads_;
};


int main(int argc, char const *argv[])
{
    printf("pid=%d, tid=%d\n", ::getpid(), CurrentThread::tid());
    
    Test t(5);
    t.run(100);

    return 0;
}
