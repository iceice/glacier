#ifndef GLACIER_BASE_LOGSTREAM_
#define GLACIER_BASE_LOGSTREAM_

#include <string.h>

#include <string>

#include "glacier/base/uncopyable.h"

namespace glacier {

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template <int SIZE>
class FixedBuffer : Uncopyable {
 public:
  FixedBuffer() : cur_(data_) { setCookie(cookieStart); }
  ~FixedBuffer() { setCookie(cookieEnd); }

  const char* data() const { return data_; }

  char* current() { return cur_; }

  int length() const { return static_cast<int>(cur_ - data_); }

  int avail() const { return static_cast<int>(end() - cur_); }

  void add(size_t len) { cur_ += len; }

  void reset() { cur_ = data_; }

  void bzero() { memset(data_, 0, sizeof data_); }

  void setCookie(void (*cookie)()) { cookie_ = cookie; }

  void append(const char* buf, size_t len) {
    if (static_cast<size_t>(avail()) > len) {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

  std::string toString() const { return std::string(data_, length()); }

 private:
  const char* end() const { return data_ + sizeof data_; }

  static void cookieStart() {}
  static void cookieEnd() {}

  void (*cookie_)();
  char data_[SIZE];
  char* cur_;
};

class LogStream : Uncopyable {
 public:
  typedef FixedBuffer<kSmallBuffer> Buffer;

  const Buffer& buffer() const { return buffer_; }

  void resetBuffer() { buffer_.reset(); }

  void append(const char* data, int len) { buffer_.append(data, len); }

  LogStream& operator<<(bool);
  LogStream& operator<<(short);
  LogStream& operator<<(unsigned short);
  LogStream& operator<<(int);
  LogStream& operator<<(unsigned int);
  LogStream& operator<<(long);
  LogStream& operator<<(unsigned long);
  LogStream& operator<<(long long);
  LogStream& operator<<(unsigned long long);
  LogStream& operator<<(const void*);
  LogStream& operator<<(float);
  LogStream& operator<<(double);
  LogStream& operator<<(long double);
  LogStream& operator<<(char);
  LogStream& operator<<(const char* str);
  LogStream& operator<<(const unsigned char* str);
  LogStream& operator<<(const std::string&);
  LogStream& operator<<(const Buffer&);

 private:
  static const int kMaxNumericSize = 32;
  
  template <typename T>
  void formatInteger(T);

  Buffer buffer_;
};

}  // namespace glacier

#endif  // GLACIER_BASE_LOGSTREAM_