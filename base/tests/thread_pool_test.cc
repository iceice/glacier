#include "base/thread_pool.h"
#include "base/count_down_latch.h"
#include "base/current_thread.h"
#include "base/logging.h"
#include "base/timestamp.h"

#include <stdio.h>
#include <unistd.h> // usleep

void print()
{
    printf("tid=%d\n", CurrentThread::tid());
}

void printString(const std::string &str)
{
    LOG_INFO << str;
    usleep(100 * 1000);
}

void test(int maxSize)
{
    Timestamp start(Timestamp::now());
    LOG_WARN << "Test ThreadPool with max queue size = " << maxSize;
    ThreadPool pool("MainThreadPool");
    pool.setMaxQueueSize(maxSize);
    pool.start(5);

    LOG_WARN << "Adding";
    pool.run(print);
    pool.run(print);
    for (int i = 0; i < 100; ++i)
    {
        char buf[32];
        snprintf(buf, sizeof buf, "task %d", i);
        pool.run(std::bind(printString, std::string(buf)));
    }
    LOG_WARN << "Done";

    CountDownLatch latch(1);
    pool.run(std::bind(&CountDownLatch::countDown, &latch));
    latch.wait();
    pool.stop();
    Timestamp end(Timestamp::now());
    printf("Use time:  %f\n", timeDifference(end, start));
}

int main()
{
    // test(0);
    // test(1);
    // test(5);
    // test(10);
    test(50);
}