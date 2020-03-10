#ifndef GLACIER_BASE_LOGSTREAM_
#define GLACIER_BASE_LOGSTREAM_

#include "glacier/base/uncopyable.h"

#include <assert.h>
#include <string>
#include <cstring>

namespace glacier {

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template <int SIZE>
class FixedBuffer : Uncopyable {
 public:
  FixedBuffer() : cur_(data_) { setCookie(cookieStart); }
  ~FixedBuffer() { setCookie(cookieEnd); }

  const char* data() const { return data_; }

  int length() const { return static_cast<int>(cur_ - data_); }

  int avail() const { return static_cast<int>(end() - cur_); }

  char* current() { return cur_; }

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
  typedef LogStream self;

  const Buffer& buffer() const { return buffer_; }

  void resetBuffer() { buffer_.reset(); }

  void append(const char* data, int len) { buffer_.append(data, len); }

  self& operator<<(bool);
  self& operator<<(short);
  self& operator<<(unsigned short);
  self& operator<<(int);
  self& operator<<(unsigned int);
  self& operator<<(long);
  self& operator<<(unsigned long);
  self& operator<<(long long);
  self& operator<<(unsigned long long);
  
  self& operator<<(const void*);
  
  self& operator<<(float);
  self& operator<<(double);
  
  self& operator<<(char);
  self& operator<<(const char* str);
  self& operator<<(const unsigned char* str);
  self& operator<<(const std::string&);
  self& operator<<(const Buffer&);

 private:
  template <typename T>
  void formatInteger(T);
  
  Buffer buffer_;

  static const int kMaxNumericSize = 32;
};

}  // namespace glacier

#endif  // GLACIER_BASE_LOGSTREAM_
