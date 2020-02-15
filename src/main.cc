#include <getopt.h>
#include <string>
#include "src/event_loop.h"
#include "src/server.h"
#include "base/logging.h"
#include "base/logfile.h"

std::unique_ptr<LogFile> g_logFile;

off_t kRollSize = 1000 * 1000 * 1000;

void outputFunc(const char *msg, int len)
{
    g_logFile->append(msg, len);
}

void flushFunc()
{
    g_logFile->flush();
}

int main(int argc, char *argv[])
{
    int threadNum = 4;
    int port = 80;

    char name[256] = {0};
    strncpy(name, argv[0], sizeof name - 1);

    g_logFile.reset(new LogFile(::basename(name), kRollSize));
    
    Logger::setOutput(outputFunc);
    Logger::setFlush(flushFunc);

    // parse args
    int opt;
    const char *str = "t:p:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 't':
        {
            threadNum = atoi(optarg);
            break;
        }
        case 'p':
        {
            port = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }


    EventLoop mainLoop;
    Server myHTTPServer(&mainLoop, threadNum, port);
    myHTTPServer.start();
    mainLoop.loop();
    return 0;
}
