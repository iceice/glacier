#ifndef BASE_FILE_UTIL_
#define BASE_FILE_UTIL_

#include "base/uncopyable.h"
#include "base/string_piece.h"

#include <sys/types.h> // for off_t

#include <string>

namespace FileUtil
{

class AppendFile : Uncopyable
{
public:
    explicit AppendFile(StringArg filename);

    ~AppendFile();

    void append(const char *logline, size_t len);

    void flush();

    off_t writtenBytes() const { return writtenBytes_; }

private:
    size_t write(const char *logline, size_t len);

    FILE *fp_;
    char buffer_[64 * 1024];
    off_t writtenBytes_;
};

}
#endif // BASE_FILE_UTIL_
