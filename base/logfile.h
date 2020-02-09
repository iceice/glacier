#ifndef BASE_LOGFILE_
#define BASE_LOGFILE_

#include "base/thread.h"
#include "base/types.h"
#include "base/file_util.h"

#include <memory>

class LogFile : Uncopyable
{
public:
    LogFile(const string &basename,
            off_t rollSize,
            bool threadSafe = true,
            int flushInterval = 3,
            int checkEveryN = 1024);
    ~LogFile();

    void append(const char *logline, int len);
    void flush();
    bool rollFile();

private:
    void append_unlocked(const char *logline, int len); //不加锁的append方式

    static string getLogFileName(const string &basename, time_t *now); //获取日志文件的名称

    const string basename_;   // 日志文件的basename
    const off_t rollSize_;    // 日志文件的大小超过rollSize_就生成一个新文件
    const int flushInterval_; // 日志写入的间隔时间
    const int checkEveryN_;

    int count_; // 行数的计数器，检测是否需要换新文件

    std::unique_ptr<MutexLock> mutex_;
    time_t startOfPeriod_; // 开始记录日志的时间
    time_t lastRoll_;      // 上一次滚动日志文件的时间
    time_t lastFlush_;     // 上一次写入日志文件的时间
    std::unique_ptr<FileUtil::AppendFile> file_; // AppendFile的智能指针

    const static int kRollPerSeconds_ = 60 * 60 * 24; // 一天时间，24小时
};

#endif // BASE_LOGFILE_
