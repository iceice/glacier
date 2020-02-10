#include "base/logging.h"
#include "base/logfile.h"
#include "base/timezone.h"

#include <stdio.h>
#include <unistd.h>

int g_total;
FILE *g_file;
std::unique_ptr<LogFile> g_logFile;

void dummyOutput(const char *msg, int len)
{
    g_total += len;
    if (g_file)
    {
        fwrite(msg, 1, len, g_file);
    }
    else if (g_logFile)
    {
        g_logFile->append(msg, len);
    }
}

void bench(const char *type)
{
    Logger::setOutput(dummyOutput);
    Timestamp start(Timestamp::now());
    g_total = 0;

    int n = 1000 * 1000;
    const bool kLongLog = false;
    string empty = " ";
    string longStr(3000, 'X');
    longStr += " ";
    for (int i = 0; i < n; ++i)
    {
        LOG_INFO << "Hello 0123456789"
                 << " abcdefghijklmnopqrstuvwxyz"
                 << (kLongLog ? longStr : empty)
                 << i;
    }
    Timestamp end(Timestamp::now());
    double seconds = timeDifference(end, start);
    printf("\n%f seconds, %d bytes, %10.2f msg/s, %.2f MiB/s\n\n\n",
           seconds, g_total, n / seconds, g_total / seconds / (1024 * 1024));
}

int main(int argc, char const *argv[])
{
    // LOG_TRACE << "trace";
    // LOG_DEBUG << "debug";
    // LOG_INFO << "Hello";
    // LOG_WARN << "World";
    // LOG_ERROR << "Error";
    // LOG_INFO << sizeof(Logger);
    // LOG_INFO << sizeof(LogStream);
    // LOG_INFO << sizeof(Fmt);
    // LOG_INFO << sizeof(LogStream::Buffer);

    bench("nop");

    return 0;
}
