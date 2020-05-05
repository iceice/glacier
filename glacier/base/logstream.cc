#include "glacier/base/logstream.h"

#include <algorithm>

namespace glacier {

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;

template <typename T>
size_t convert(char buf[], T value) {
  T i = value;
  char* p = buf;

  do {
    int lsd = static_cast<int>(i % 10);
    i /= 10;
    *p++ = zero[lsd];
  } while (i != 0);

  if (value < 0) *p++ = '-';

  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

const char digitsHex[] = "0123456789ABCDEF";

size_t convertHex(char buf[], uintptr_t value) {
  uintptr_t i = value;
  char* p = buf;

  do {
    int lsd = static_cast<int>(i % 16);
    i /= 16;
    *p++ = digitsHex[lsd];
  } while (i != 0);

  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

template <typename T>
void LogStream::formatInteger(T v) {
  if (buffer_.avail() >= kMaxNumericSize) {
    // buffer剩余容量大于等于kMaxNumericSize
    size_t len = convert(buffer_.current(), v);
    buffer_.add(len);
  }
}

LogStream& LogStream::operator<<(bool v) {
  buffer_.append(v ? "1" : "0", 1);
  return *this;
}

LogStream& LogStream::operator<<(short v) {
  *this << static_cast<int>(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned short v) {
  *this << static_cast<unsigned int>(v);
  return *this;
}

LogStream& LogStream::operator<<(int v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long long v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(const void* p) {
  if (buffer_.avail() >= kMaxNumericSize) {
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    char* buf = buffer_.current();
    buf[0] = '0';
    buf[1] = 'x';
    size_t len = convertHex(buf + 2, v);
    buffer_.add(2 + len);
  }
  return *this;
}

LogStream& LogStream::operator<<(float v) {
  *this << static_cast<double>(v);
  return *this;
}

LogStream& LogStream::operator<<(double v) {
  if (buffer_.avail() >= kMaxNumericSize) {
    int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
    buffer_.add(len);
  }
  return *this;
}

LogStream& LogStream::operator<<(char v) {
  buffer_.append(&v, 1);
  return *this;
}

LogStream& LogStream::operator<<(const char* str) {
  if (str)
    buffer_.append(str, strlen(str));
  else
    buffer_.append("(null)", 6);
  return *this;
}

LogStream& LogStream::operator<<(const unsigned char* str) {
  *this << reinterpret_cast<const char*>(str);
  return *this;
}

LogStream& LogStream::operator<<(const std::string& v) {
  buffer_.append(v.c_str(), v.size());
  return *this;
}

LogStream& LogStream::operator<<(const Buffer& v) {
  *this << v.toString();
  return *this;
}

}  // namespace glacier