#ifndef GLACIER_BASE_LOGGING_
#define GLACIER_BASE_LOGGING_

#include <stdint.h>
#include <string.h>

#include "glacier/base/logstream.h"
#include "glacier/base/timestamp.h"
#include "glacier/base/timezone.h"

namespace glacier {

class Logger {
 public:
  enum LogLevel {
    TRACE = 0,           // 细粒度最高的信息
    DEBUG = 1,           // 对调试有帮助的事件信息
    INFO = 2,            // 粗粒度级别上强调程序的运行信息
    WARN = 3,            // 程序能正常运行，但是有潜在危险的信息
    ERROR = 4,           // 程序出错，但是不影响系统运行的信息
    FATAL = 5,           // 将导致程序停止运行的严重信息
    NUM_LOG_LEVELS = 6,  // 日志级别个数
  };

  typedef Logger::LogLevel LogLevel;
  typedef void (*OutputFunc)(const char* msg, int len);
  typedef void (*FlushFunc)();

  // SourceFile帮助我们在编译期获得__FILE__的 basename
  class SourceFile {
   public:
    template <int N>
    SourceFile(const char (&arr)[N]) : data_(arr), size_(N) {
      // 得到最后一个'/'符号之后的单词，即 basename
      const char* slash = strrchr(data_, '/');
      if (slash) {
        data_ = slash + 1;
        size_ -= static_cast<int>(data_ - arr);
      }
    }

    explicit SourceFile(const char* filename) : data_(filename) {
      // 得到最后一个'/'符号之后的单词，即 basename
      const char* slash = strrchr(filename, '/');
      if (slash) data_ = slash + 1;
      size_ = static_cast<int>(strlen(data_));
    }

   public:
    const char* data_;
    int size_;
  };

  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char* func);
  ~Logger();

  static void setLogLevel(LogLevel level);
  static LogLevel logLevel();

  static void setOutput(OutputFunc);
  static void setFlush(FlushFunc);
  static void setTimeZone(const TimeZone& tz);

  LogStream& stream() { return impl_.stream_; }

 private:
  // Private Implementation机制，用来减少不必要的编译
  class Impl {
   public:
    Impl(LogLevel level, const SourceFile& file, int line);

    void finish();
    void formatTime();

   public:
    LogStream stream_;
    LogLevel level_;
    SourceFile basename_;
    int line_;
    Timestamp time_;
  };

  Impl impl_;
};

extern Logger::LogLevel g_loglevel;  // 定义一个全局的日志级别

inline Logger::LogLevel Logger::logLevel() { return g_loglevel; }

#define LOG_TRACE                                            \
  if (glacier::Logger::logLevel() <= glacier::Logger::TRACE) \
  glacier::Logger(__FILE__, __LINE__, glacier::Logger::TRACE, __func__).stream()
#define LOG_DEBUG                                            \
  if (glacier::Logger::logLevel() <= glacier::Logger::DEBUG) \
  glacier::Logger(__FILE__, __LINE__, glacier::Logger::DEBUG, __func__).stream()
#define LOG_INFO                                            \
  if (glacier::Logger::logLevel() <= glacier::Logger::INFO) \
  glacier::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN  glacier::Logger(__FILE__, __LINE__, glacier::Logger::WARN).stream()
#define LOG_ERROR glacier::Logger(__FILE__, __LINE__, glacier::Logger::ERROR).stream()
#define LOG_FATAL glacier::Logger(__FILE__, __LINE__, glacier::Logger::FATAL).stream()

}  // namespace glacier

#endif // GLACIER_BASE_LOGGING_
