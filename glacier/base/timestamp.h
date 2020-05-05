#ifndef GLACIER_BASE_TIMESTAMP_
#define GLACIER_BASE_TIMESTAMP_

#include <boost/operators.hpp>

namespace glacier {

class Timestamp : public boost::equality_comparable<Timestamp>,
                  public boost::less_than_comparable<Timestamp> {
 public:
  // Constucts an invalid Timestamp.
  Timestamp() : microSecondsSinceEpoch_(0) {}

  // Constucts a Timestamp at specific time
  explicit Timestamp(int64_t microSecondsSinceEpochArg)
      : microSecondsSinceEpoch_(microSecondsSinceEpochArg) {}

  static const int kMicroSecondsPerSecond = 1000 * 1000;

  std::string toString() const;
  std::string toFormattedString(bool showMicroseconds = true) const;

  bool valid() const { return microSecondsSinceEpoch_ > 0; }

  // for internal usage.
  int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
  time_t secondsSinceEpoch() const {
    return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
  }

  // Get time of now.
  static Timestamp now();
  static Timestamp invalid() { return Timestamp(); }

  static Timestamp fromUnixTime(time_t t) { return fromUnixTime(t, 0); }
  static Timestamp fromUnixTime(time_t t, int microseconds) {
    return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
  }

 private:
  int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs) {
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

// Gets time difference of two timestamps, result in seconds.
inline double timeDifference(Timestamp high, Timestamp low) {
  int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
  return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

// Add seconds to given timestamp.
inline Timestamp addTime(Timestamp timestamp, double seconds) {
  int64_t delta =
      static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
  return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

}  // namespace glacier

#endif  // GLACIER_BASE_TIMESTAMP_