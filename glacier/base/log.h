#ifndef GLACIER_BASE_LOG_
#define GLACIER_BASE_LOG_

#include "glacier/base/logstream.h"

#include <stdint.h>
#include <cstring>
#include <memory>


#define LOG_TRACE if (glacier::Logger::logLevel() <= glacier::Logger::TRACE) \
  glacier::Logger(__FILE__, __LINE__, glacier::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (glacier::Logger::logLevel() <= glacier::Logger::DEBUG) \
  glacier::Logger(__FILE__, __LINE__, glacier::Logger::DEBUG, __func__).stream()
#define LOG_INFO  if (glacier::Logger::logLevel() <= glacier::Logger::INFO)  \
  glacier::Logger(__FILE__, __LINE__, glacier::Logger::INFO, __func__).stream()
#define LOG_WARN  glacier::Logger(__FILE__, __LINE__, glacier::Logger::WARN).stream();
#define LOG_ERROR glacier::Logger(__FILE__, __LINE__, glacier::Logger::ERROR).stream();
#define LOG_FATAL glacier::Logger(__FILE__, __LINE__, glacier::Logger::FATAL).stream();

namespace glacier {

class Logger {
 public:
  //
  // 日志的级别，级别越低输出的信息越多
  // 只会输出大于等于 g_loglevel 级别的日志信息。
  //
  // 例如：
  //   g_loglevel = LogLevel::INFO;
  //   那么 TRACE 和 DEBUG 的信息就不会输出
  //
  enum LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5,
    NUM_LOG_LEVELS = 6,
  };

  //
  // 内部类 SourceFile
  // 帮助我们在编译期获得__FILE__的 basename
  //
  class SourceFile {
   public:
    //
    // 构造函数，传入参数是一个保留了
    // 大小信息的 char 数组的引用
    //
    template <int N>
    SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1) {
      // 得到最后一个'/'符号之后的单词，即 basename
      const char* slash = strrchr(data_, '/');
      if (slash) {
        data_ = slash + 1;
        size_ -= static_cast<int>(data_ - arr);
      }
    }
    //
    // 构造函数，传入参数为指向char的指针
    // 禁止隐式转换
    //
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

  typedef std::shared_ptr<Logger> ptr;
  typedef Logger::LogLevel LogLevel;
  typedef void (*OutputFunc)(const char* msg, int len);
  typedef void (*FlushFunc)();

  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char* func);
  ~Logger();

  static void SetLogLevel(LogLevel level);
  static LogLevel logLevel();

  static void SetOutput(OutputFunc);
  static void SetFlush(FlushFunc);

  LogStream& stream() { return impl_.stream_; }

 private:
  //
  // 内部类 Impl
  // Private Implementation机制，用来减少不必要的编译
  //
  class Impl {
   public:
    Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
    void finish();

    LogStream stream_;
    LogLevel level_;
    SourceFile basename_;
    int line_;
    uint64_t time_ = 0;
  };

  Impl impl_;
};

extern Logger::LogLevel g_loglevel;  // 定义一个全局的日志级别

inline Logger::LogLevel Logger::logLevel() {
  return g_loglevel;
}

}  // namespace glacier

#endif  // GLACIER_BASE_LOG_
