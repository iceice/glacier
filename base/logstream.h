#ifndef BASE_LOGSTREAM_
#define BASE_LOGSTREAM_

#include "base/uncopyable.h"
#include "base/string_piece.h"
#include "base/types.h"

#include <assert.h>
#include <string.h> // memcpy

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template <int SIZE>
class FixedBuffer : private Uncopyable
{
public:
    FixedBuffer() : cur_(data_) { setCookie(cookieStart); }

    ~FixedBuffer() { setCookie(cookieEnd); }

    void append(const char *buf, size_t len)
    {
        if (implicit_cast<size_t>(avail()) > len)
        {
            // 如果可用数据足够，就拷贝过去，同时移动当前指针。
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    const char *data() const { return data_; }                    // 返回数据，即首地址
    int length() const { return static_cast<int>(cur_ - data_); } // 返回缓冲区已有数据长度

    // write to data_ directly
    char *current() { return cur_; }                             // 返回当前数据末端地址
    int avail() const { return static_cast<int>(end() - cur_); } // 返回剩余可用地址
    void add(size_t len) { cur_ += len; }                        // cur前移

    void reset() { cur_ = data_; }                 // 重置，不清除数据，只需要让cur指回首地址即可
    void bzero() { memZero(data_, sizeof data_); } // 数据清空

    // for used by GDB
    const char *debugString();
    void setCookie(void (*cookie)()) { cookie_ = cookie; }

    // for used by unit test
    string toString() const { return string(data_, length()); }
    StringPiece toStringPiece() const { return StringPiece(data_, length()); }

private:
    const char *end() const { return data_ + sizeof data_; } // 返回末尾指针
    // Must be outline function for cookies.
    static void cookieStart();
    static void cookieEnd();
    void (*cookie_)();

    char data_[SIZE]; // 缓冲区数组
    char *cur_;       // 指向当前数据的末尾
};

class LogStream : private Uncopyable
{
    typedef LogStream self;

public:
    typedef FixedBuffer<kSmallBuffer> Buffer;

    self &operator<<(bool v)
    {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }

    self &operator<<(short);
    self &operator<<(unsigned short);
    self &operator<<(int);
    self &operator<<(unsigned int);
    self &operator<<(long);
    self &operator<<(unsigned long);
    self &operator<<(long long);
    self &operator<<(unsigned long long);

    self &operator<<(const void *);

    self &operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }
    self &operator<<(double);

    self &operator<<(char v)
    {
        buffer_.append(&v, 1);
        return *this;
    }

    self &operator<<(const char *str)
    {
        if (str)
        {
            buffer_.append(str, strlen(str));
        }
        else
        {
            buffer_.append("(null)", 6);
        }
        return *this;
    }

    self &operator<<(const unsigned char *str)
    {
        return operator<<(reinterpret_cast<const char *>(str));
    }

    self &operator<<(const string &v)
    {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }

    self &operator<<(const StringPiece &v)
    {
        buffer_.append(v.data(), v.size());
        return *this;
    }

    self &operator<<(const Buffer &v)
    {
        *this << v.toStringPiece();
        return *this;
    }

    void append(const char *data, int len) { buffer_.append(data, len); }
    const Buffer &buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); }

private:
    void staticCheck();

    template <typename T>
    void formatInteger(T);

    Buffer buffer_;

    static const int kMaxNumericSize = 32;
};

class Fmt // : noncopyable
{
public:
    template <typename T>
    Fmt(const char *fmt, T val);

    const char *data() const { return buf_; }
    int length() const { return length_; }

private:
    char buf_[32];
    int length_;
};

inline LogStream &operator<<(LogStream &s, const Fmt &fmt)
{
    s.append(fmt.data(), fmt.length());
    return s;
}

#endif // BASE_LOGSTREAM_