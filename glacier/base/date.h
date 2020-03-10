#ifndef GLACIER_BASE_DATE_
#define GLACIER_BASE_DATE_

#include <stdint.h>
#include <time.h>
#include <algorithm>

namespace glacier {

class Date {
 public:
  struct YearMonthDay {
    int year;   // [1900..2500]
    int month;  // [1..12]
    int day;    // [1..31]
  };

  static const int kDaysPerWeek = 7;
  static const int kJulianDayOf1970_01_01;

  // 构造函数
  Date() : julianDayNumber_(0) {}
  Date(int year, int month, int day);
  explicit Date(int julianDayNum) : julianDayNumber_(julianDayNum) {}
  explicit Date(const struct tm&);

  // 判断日期是否有效
  bool valid() const { return julianDayNumber_ > 0; }

  // 将日期转为yyyy-mm-dd的格式
  std::string toIsoString() const;

  struct YearMonthDay yearMonthDay() const;

  int year() const { return yearMonthDay().year; }
  int month() const { return yearMonthDay().month; }
  int day() const { return yearMonthDay().day; }

  // [0, 1, ..., 6] => [Sunday, Monday, ..., Saturday ]
  int weekDay() const { return (julianDayNumber_ + 1) % kDaysPerWeek; }

  int julianDayNumber() const { return julianDayNumber_; }

 private:
  int julianDayNumber_;
};

inline bool operator<(Date x, Date y) {
  return x.julianDayNumber() < y.julianDayNumber();
}

inline bool operator==(Date x, Date y) {
  return x.julianDayNumber() == y.julianDayNumber();
}

}  // namespace glacier

#endif  // GLACIER_BASE_DATE_
