#include "base/file_util.h"
#include "base/logging.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

FileUtil::AppendFile::AppendFile(StringArg filename)
    : fp_(::fopen(filename.c_str(), "ae")),
      writtenBytes_(0)
{
    assert(fp_);
    ::setbuffer(fp_, buffer_, sizeof buffer_);
}

FileUtil::AppendFile::~AppendFile()
{
    ::fclose(fp_);
}

void FileUtil::AppendFile::append(const char *logline, const size_t len)
{
    size_t n = write(logline, len);
    size_t remain = len - n;
    while (remain > 0)
    {
        size_t x = write(logline + n, remain);
        if (x == 0)
        {
            int err = ferror(fp_);
            if (err)
            {
                fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tl(err));
            }
            break;
        }
        n += x;
        remain = len - n; // remain -= x
    }

    writtenBytes_ += len;
}

void FileUtil::AppendFile::flush()
{
    ::fflush(fp_);
}

size_t FileUtil::AppendFile::write(const char *logline, size_t len)
{
    // #undef fwrite_unlocked
    return ::fwrite_unlocked(logline, 1, len, fp_);
}