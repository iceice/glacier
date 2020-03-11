#include "glacier/base/log.h"
#include <time.h>
#include "glacier/base/logfile.h"
#include "glacier/base/threadpool.h"
#include "glacier/base/timestamp.h"

using namespace glacier;

int g_total = 0;
FILE* g_file = nullptr;
LogFile::ptr g_logFile = nullptr;

void logInThread() {
  LOG_INFO << "logInThread";
  usleep(1000);
}

void dummyOutput(const char* msg, int len) {
  g_total += len;
  if (g_file) {
    fwrite(msg, 1, len, g_file);
  } else if (g_logFile) {
    g_logFile->append(msg, len);
  }
}

void bench(const char* type) {
  Logger::setOutput(dummyOutput);
  Timestamp start(Timestamp::now());
  g_total = 0;

  int n = 1000 * 1000;
  const bool kLongLog = false;
  std::string empty = " ";
  std::string longStr(3000, 'X');
  longStr += " ";
  for (int i = 0; i < n; ++i) {
    LOG_INFO << "Hello 0123456789"
             << " abcdefghijklmnopqrstuvwxyz" << (kLongLog ? longStr : empty)
             << i;
  }
  Timestamp end(Timestamp::now());
  double seconds = timeDifference(end, start);
  printf("%12s: %f seconds, %d bytes, %10.2f msg/s, %.2f MiB/s\n", type,
         seconds, g_total, n / seconds, g_total / seconds / (1024 * 1024));
}

int main(int argc, char const* argv[]) {
  // ThreadPool pool(4);
  // pool.commit(logInThread);
  // pool.commit(logInThread);
  // pool.commit(logInThread);
  // pool.commit(logInThread);

  // LOG_TRACE << "trace";
  // LOG_DEBUG << "debug";
  // LOG_INFO << "Hello";
  // LOG_WARN << "World";
  // LOG_ERROR << "Error";
  // LOG_INFO << sizeof(glacier::Logger);
  // LOG_INFO << sizeof(glacier::LogStream);
  // LOG_INFO << sizeof(glacier::LogStream::Buffer);

  sleep(1);
  bench("nop");

  return 0;
}
