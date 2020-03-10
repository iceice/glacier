#include "glacier/base/log.h"

namespace glacier {

__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastsecond;

void DefaultOutput(const char* msg, int len) { fwrite(msg, 1, len, stdout); }

void DefaultFlush() { fflush(stdout); }

Logger::LogLevel g_loglevel = Logger::INFO;
Logger::OutputFunc g_output = DefaultOutput;
Logger::FlushFunc g_flush = DefaultFlush;

const char* LogLevelName[Logger::NUM_LOG_LEVELS] = {
    "TRACE ", "DEBUG ", "INFO  ", "WARN  ", "ERROR ", "FATAL ",
};

Logger::Logger(SourceFile file, int line) : impl_(INFO, 0, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level)
    : impl_(level, 0, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
    : impl_(level, 0, file, line) {
  impl_.stream_ << func << ' ';
}

Logger::~Logger() {
  impl_.finish();
  const LogStream::Buffer& buf(stream().buffer());
  g_output(buf.data(), buf.length());
  if (impl_.level_ == FATAL) {
    g_flush();
    abort();
  }
}

void Logger::SetLogLevel(Logger::LogLevel level) { g_loglevel = level; }

inline Logger::LogLevel Logger::logLevel() { return g_loglevel; }

void Logger::SetOutput(OutputFunc out) { g_output = out; }

void Logger::SetFlush(FlushFunc flush) { g_flush = flush; }

}  // namespace glacier