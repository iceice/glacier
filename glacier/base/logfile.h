#ifndef GLACIER_BASE_LOGFILE_
#define GLACIER_BASE_LOGFILE_

#include <memory>
#include <mutex>
#include <string>

#include "glacier/base/uncopyable.h"

namespace glacier {

const int BUFFER_SIZE = 64 * 1024;

//
// AppendFile - 管理文件指针
//
class AppendFile : Uncopyable {
 public:
  typedef std::shared_ptr<AppendFile> ptr;

  explicit AppendFile(std::string filename);

  ~AppendFile();

  void append(const char* logline, size_t len);

  void flush();

  size_t writtenBytes() const { return writtenBytes_; }

 private:
  size_t write(const char* logline, size_t len);

  FILE* fd_;                  // 文件指针
  char buffer_[BUFFER_SIZE];  // 缓冲区
  size_t writtenBytes_;       // 已经写入字符的偏移量
};

class LogFile : Uncopyable {
 public:
  typedef std::shared_ptr<LogFile> ptr;

  LogFile(const std::string& name, size_t rollSize);
  LogFile(const std::string& name, size_t rollSize, int flushInterval, int checkEveryN);

  ~LogFile();

  void append(const char* logline, size_t len);

  void flush();

  bool rollFile();

 private:
  static std::string getLogFileName(const std::string& basename, time_t* now);

  const std::string basename_;  // 日志文件的名字
  const size_t rollSize_;       // 日志文件大小达到rollSize生成下一个文件
  const int flushInterval_;     // 日志写入间隔时间
  const int checkEveryN_;       // 每写入checkEveryN行数据，检测是否需要写入磁盘

  int count_;  // 计数器

  std::mutex mutex_;
  time_t startOfPeriod_;  // 开始记录日志的时间
  time_t lastRoll_;       // 上一次滚动日志的时间
  time_t lastFlush_;      // 上一次写入磁盘的时间

  AppendFile::ptr file_;  // 文件指针

  const static int kRollPerSeconds_ = 60 * 60 * 24;  // 一天时间
};

}  // namespace glacier

#endif  // GLACIER_BASE_LOGFILE_