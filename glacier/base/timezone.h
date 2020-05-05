#ifndef GLACIER_BASE_TIMEZONE_
#define GLACIER_BASE_TIMEZONE_

#include <time.h>

#include <memory>

namespace glacier {

class TimeZone {
 public:
  TimeZone() = default;
  TimeZone(int eastOfUtc, const char* tzname);
  explicit TimeZone(const char* zonefile);

  bool valid() const { return static_cast<bool>(data_); }

  struct tm toLocalTime(time_t secondsSinceEpoch) const;
  time_t fromLocalTime(const struct tm&) const;

  static struct tm toUtcTime(time_t secondsSinceEpoch, bool yday = false);
  static time_t fromUtcTime(const struct tm&);
  static time_t fromUtcTime(int year, int month, int day, int hour, int minute, int seconds);
  struct Data;

 private:
  std::shared_ptr<Data> data_;
};

}  // namespace glacier

#endif  // GLACIER_BASE_TIMEZONE_