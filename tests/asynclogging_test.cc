#include "glacier/base/asynclogging.h"
#include "glacier/base/log.h"
#include "glacier/base/timestamp.h"

#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

using namespace glacier;

size_t kRollSize = 500 * 1000 * 1000;

AsyncLogging* g_asyncLog;

void asyncOutput(const char* msg, int len) { g_asyncLog->append(msg, len); }

void bench(bool longLog) {
  int cnt = 0;
  const int kBatch = 1000;
  string empty = " ";
  string longStr(3000, 'x');
  longStr += " ";

  for (int t = 0; t < 30; ++t) {
    Timestamp start = Timestamp::now();
    for (int i = 0; i < kBatch; ++i) {
      LOG_INFO << "Hello 0123456789"
               << " abcdefghijklmnopqrstuvwxyz " << (longLog ? longStr : empty)
               << cnt;
      ++cnt;
    }
    Timestamp end = Timestamp::now();
    printf("%f\n", timeDifference(end, start) * 1000000 / kBatch);
    struct timespec ts = {0, 500 * 1000 * 1000};
    nanosleep(&ts, NULL);
  }
}

int main(int argc, char const* argv[]) {
  {
    // set max virtual memory to 2GB.
    size_t kOneGB = 1000 * 1024 * 1024;
    rlimit rl = {2 * kOneGB, 2 * kOneGB};
    setrlimit(RLIMIT_AS, &rl);
  }

  printf("pid = %d\n", getpid());

  char name[256] = {0};
  strncpy(name, argv[0], sizeof(name) - 1);
  AsyncLogging log(::basename(name), kRollSize, 3);
  log.start();
  g_asyncLog = &log;

  Logger::setOutput(asyncOutput);

  bool longLog = argc > 1;
  bench(longLog);

  return 0;
}
