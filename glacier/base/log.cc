#include "glacier/base/log.h"

#include "glacier/base/current_thread.h"

#define UNUSED(x) (void)(x)

namespace glacier {

__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastsecond;

void DefaultOutput(const char* msg, int len) { fwrite(msg, 1, len, stdout); }

void DefaultFlush() { fflush(stdout); }

Logger::LogLevel g_loglevel = Logger::INFO;
Logger::OutputFunc g_output = DefaultOutput;
Logger::FlushFunc g_flush   = DefaultFlush;
TimeZone g_logTimeZone;

const char* LogLevelName[Logger::NUM_LOG_LEVELS] = {
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
};

/*
 * 辅助类，在编译器获得字符串的长度
 */
class T {
 public:
  T(const char* str, unsigned len) : str_(str), len_(len) {}

  const char* str_;
  const unsigned len_;
};

class Fmt {
 public:
  template <typename T>
  Fmt(const char* fmt, T val) {
    length_ = snprintf(buf_, sizeof buf_, fmt, val);
  }

  const char* data() const { return buf_; }
  int length() const { return length_; }

 private:
  char buf_[32];
  int length_;
};

inline LogStream& operator<<(LogStream& s, T v) {
  s.append(v.str_, v.len_);
  return s;
}

inline LogStream& operator<<(LogStream& s, const Fmt& fmt) {
  s.append(fmt.data(), fmt.length());
  return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v) {
  s.append(v.data_, v.size_);
  return s;
}

Logger::Impl::Impl(LogLevel level, const SourceFile& file, int line)
    : stream_(),
      level_(level),
      basename_(file),
      line_(line),
      time_(Timestamp::now()) {
  formatTime();
  CurrentThread::tid();
  stream_ << T(CurrentThread::tidString(), CurrentThread::tidStringLength());
  stream_ << T(LogLevelName[level], 6);
}

void Logger::Impl::finish() {
  stream_ << " - " << basename_ << ":" << line_ << "\n";
}

void Logger::Impl::formatTime() {
  int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();

  time_t seconds   = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::kMicroSecondsPerSecond);
  int microseconds = static_cast<int>(microSecondsSinceEpoch % Timestamp::kMicroSecondsPerSecond);
  if (seconds != t_lastsecond) {
    t_lastsecond = seconds;
    struct tm tm_time;
    if (g_logTimeZone.valid())
      tm_time = g_logTimeZone.toLocalTime(seconds);
    else
      ::gmtime_r(&seconds, &tm_time);

    int len = snprintf(t_time, sizeof(t_time), "%4d-%02d-%02d %02d:%02d:%02d",
                       tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                       tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    assert(len == 19);
    UNUSED(len);
  }
  if (g_logTimeZone.valid()) {
    Fmt us(":%06d ", microseconds);
    assert(us.length() == 8);
    stream_ << T(t_time, 19) << T(us.data(), 8);
  } else {
    Fmt us(":%06dZ ", microseconds);
    assert(us.length() == 9);
    stream_ << T(t_time, 19) << T(us.data(), 9);
  }
}

Logger::Logger(SourceFile file, int line) : impl_(INFO, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level) : impl_(level, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func) : impl_(level, file, line) {
  impl_.stream_ << func << ' ';
}

Logger::~Logger() {
  impl_.finish();
  const LogStream::Buffer& buf(this->stream().buffer());
  g_output(buf.data(), buf.length());
  if (impl_.level_ == FATAL) {
    g_flush();
    abort();
  }
}

void Logger::setLogLevel(Logger::LogLevel level) { g_loglevel = level; }

void Logger::setOutput(OutputFunc out) { g_output = out; }

void Logger::setFlush(FlushFunc flush) { g_flush = flush; }

void Logger::setTimeZone(const TimeZone& tz) { g_logTimeZone = tz; }

}  // namespace glacier