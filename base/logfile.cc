#include "logfile.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>

LogFile::LogFile(const string &basename,
                 off_t rollSize,
                 bool threadSafe,
                 int flushInterval,
                 int checkEveryN)
    : basename_(basename),
      rollSize_(rollSize),
      flushInterval_(flushInterval),
      checkEveryN_(checkEveryN),
      count_(0),
      mutex_(threadSafe ? new MutexLock : NULL),
      startOfPeriod_(0),
      lastRoll_(0),
      lastFlush_(0)
{
    assert(basename.find('/') == string::npos); //断言basename不包含'/'
    rollFile();
}

LogFile::~LogFile() = default;

// 向日志中追加文本
void LogFile::append(const char *logline, int len)
{
    if (mutex_)
    {
        // 线程安全模式，效率低
        MutexLockGuard lock(*mutex_);
        append_unlocked(logline, len);
    }
    else
    {
        // 非线程安全模式，效率高
        append_unlocked(logline, len);
    }
}

// 写入磁盘
void LogFile::flush()
{
    if (mutex_)
    {
        MutexLockGuard lock(*mutex_);
        file_->flush();
    }
    else
    {
        file_->flush();
    }
}

// 不加锁的追加方式，程序的主要调用接口
void LogFile::append_unlocked(const char *logline, int len)
{
    file_->append(logline, len); // 调用AppendFile的append方法

    if (file_->writtenBytes() > rollSize_)
    {
        // 写入字节数大小超过滚动设定大小，则滚动文件
        rollFile();
    }
    else
    {
        ++count_; // 行数加1
        if (count_ >= checkEveryN_)
        {
            // 检查是否需要写磁盘，或者滚动文件
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
            if (thisPeriod_ != startOfPeriod_)
            {
                // 每天0点自动新建文件
                rollFile();
            }
            else if (now - lastFlush_ > flushInterval_)
            {
                // 每 flushInterval_ (eg.3)秒写入一次磁盘
                lastFlush_ = now;
                file_->flush();
            }
        }
    }
}

bool LogFile::rollFile()
{
    time_t now = 0;
    string filename = getLogFileName(basename_, &now);        // 生成一个文件名
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_; // 以一天为单位

    if (now > lastRoll_)
    {
        // 如果当前时间大于上次滚动的时间
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new FileUtil::AppendFile(filename));
        return true;
    }
    return false;
}

string LogFile::getLogFileName(const string &basename, time_t *now)
{
    string filename;
    filename.reserve(basename.size() + 64); //预分配内存
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    gmtime_r(now, &tm);                                       // 将当前时间转为UTC时间，并存储到tm中
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S", &tm); //格式化时间
    filename += timebuf;
    filename += ".log";

    return filename;
}
