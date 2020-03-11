#ifndef GLACIER_BASE_ASYNCLOG_
#define GLACIER_BASE_ASYNCLOG_

#include "glacier/base/countdown.h"
#include "glacier/base/logstream.h"
#include "glacier/base/uncopyable.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

using std::string;

namespace glacier {

class AsyncLogging : Uncopyable {
 public:
  typedef std::shared_ptr<AsyncLogging> ptr;

  AsyncLogging(const string& basename, off_t rollSize, int flushInterval);

  ~AsyncLogging();

  void append(const char* logline, int len);

  void start();

  void stop();

 private:
  typedef FixedBuffer<kLargeBuffer> Buffer;
  typedef std::vector<std::unique_ptr<Buffer> > BufferVector;
  typedef BufferVector::value_type BufferPtr;
  typedef std::thread Thread;

  void threadFunc();

  const string basename_;  // 日志文件的名字
  const size_t rollSize_;  // 日志文件大小达到rollSize生成下一个文件
  const int flushInterval_;            // 日志写入间隔时间
  std::atomic<bool> running_;          // 运行标志
  std::mutex mutex_;                   // 互斥量
  std::condition_variable condition_;  // 条件变量
  Thread thread_;                      // 主线程
  CountDownLatch latch_;               // 计数器

  BufferPtr currentBuffer_;  // 当前缓冲
  BufferPtr nextBuffer_;     // 下一个缓冲

  BufferVector buffers_;  // 缓冲列表
};

}  // namespace glacier

#endif  // GLACIER_BASE_ASYNCLOG_