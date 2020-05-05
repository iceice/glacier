#include "glacier/base/logfile.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <iostream>

using namespace glacier;

typedef std::lock_guard<std::mutex> MutexLockGuard;

std::string hostname() {
  char buf[256];
  if (gethostname(buf, sizeof buf) == 0) {
    buf[sizeof(buf) - 1] = '\0';
    return buf;
  } else {
    return "unknownhost";
  }
}

AppendFile::AppendFile(std::string filename) : fd_(fopen(filename.c_str(), "ae")), writtenBytes_(0) {}

AppendFile::~AppendFile() { fclose(fd_); }

void AppendFile::append(const char* logline, size_t len) {
  size_t n = write(logline, len);
  size_t remain = len - n;
  while (remain > 0) {
    size_t x = write(logline + n, remain);
    n += x;
    remain -= x;
  }
  writtenBytes_ += len;
}

void AppendFile::flush() { fflush(fd_); }

size_t AppendFile::write(const char* logline, size_t len) {
  return fwrite_unlocked(logline, 1, len, fd_);
}

LogFile::LogFile(const std::string& name, size_t rollSize)
    : basename_(name),
      rollSize_(rollSize),
      flushInterval_(3),
      checkEveryN_(1024),
      count_(0),
      startOfPeriod_(0),
      lastRoll_(0),
      lastFlush_(0) {
  assert(name.find('/') == std::string::npos);
  rollFile();
}

LogFile::LogFile(const std::string& name, size_t rollSize, int flushInterval, int checkEveryN)
    : basename_(name),
      rollSize_(rollSize),
      flushInterval_(flushInterval),
      checkEveryN_(checkEveryN),
      count_(0),
      startOfPeriod_(0),
      lastRoll_(0),
      lastFlush_(0) {
  assert(name.find('/') == std::string::npos);
  rollFile();
}

LogFile::~LogFile() {}

void LogFile::append(const char* logline, size_t len) {
  MutexLockGuard lock(mutex_);
  file_->append(logline, len);
  if (file_->writtenBytes() > rollSize_) {
    // 写入的字符超过了回滚大小
    rollFile();
  } else {
    if (++count_ >= checkEveryN_) {
      // 检查时间是否达到第二天
      count_ = 0;
      time_t now = ::time(0);
      time_t cur = now / kRollPerSeconds_ * kRollPerSeconds_;
      if (cur != startOfPeriod_) {
        rollFile();
      } else if (now - lastFlush_ > flushInterval_) {
        lastFlush_ = now;
        file_->flush();
      }
    }
  }
}

void LogFile::flush() {
  MutexLockGuard lock(mutex_);
  file_->flush();
}

bool LogFile::rollFile() {
  time_t now = 0;
  std::string filename = getLogFileName(basename_, &now);
  time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

  if (now > lastRoll_) {
    lastRoll_ = now;
    lastFlush_ = now;
    startOfPeriod_ = start;
    file_.reset(new AppendFile(filename));
    return true;
  }
  return false;
}

std::string LogFile::getLogFileName(const std::string& basename, time_t* now) {
  std::string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;

  char timebuf[32];
  struct tm tm;
  *now = time(0);
  gmtime_r(now, &tm);
  strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
  filename += timebuf;

  filename += hostname();

  char pidbuf[32];
  snprintf(pidbuf, sizeof pidbuf, ".%d", ::getpid());
  filename += pidbuf;

  filename += ".log";
  return filename;
}