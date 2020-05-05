#include "glacier/base/asynclogging.h"

#include <assert.h>

#include "glacier/base/logfile.h"
#include "glacier/base/timestamp.h"

using namespace glacier;

typedef std::lock_guard<std::mutex> MutexLockGuard;

AsyncLogging::AsyncLogging(const string& basename, off_t rollSize, int flushInterval)
    : basename_(basename),
      rollSize_(rollSize),
      flushInterval_(flushInterval),
      running_(false),
      mutex_(),
      condition_(),
      thread_(),
      latch_(1),
      currentBuffer_(new Buffer),
      nextBuffer_(new Buffer),
      buffers_() {
  currentBuffer_->bzero();
  nextBuffer_->bzero();
  buffers_.reserve(16);
}

AsyncLogging::~AsyncLogging() {
  if (running_) stop();
}

void AsyncLogging::start() {
  running_.store(true);
  thread_ = Thread(std::bind(&AsyncLogging::threadFunc, this));
  latch_.wait();  // 确保线程启动
}

void AsyncLogging::stop() {
  running_.store(false);
  condition_.notify_all();
  thread_.join();
}

void AsyncLogging::append(const char* logline, int len) {
  MutexLockGuard lock(mutex_);  // 确保线程安全
  if (currentBuffer_->avail() > len) {
    // 当前buffer的可用空间足够
    currentBuffer_->append(logline, len);
  } else {
    // 当前buffer的空间不足，则把当前buffer移入列表
    buffers_.push_back(std::move(currentBuffer_));

    if (nextBuffer_) {
      // 如果有下一个缓冲区，则赋值为当前缓冲区
      currentBuffer_ = std::move(nextBuffer_);
    } else {
      // 前端写入速度太快，两块缓冲都用完了，分配一块新的buffer
      currentBuffer_.reset(new Buffer);  // Rarely happens
    }
    currentBuffer_->append(logline, len);
    condition_.notify_all();  // 通知日志线程，有数据可写
  }
}

void AsyncLogging::threadFunc() {
  assert(running_ == true);
  latch_.countDown();
  LogFile output(basename_, rollSize_);  // 初始化日志文件
  BufferPtr newBuffer1(new Buffer);      // 初始化两个buffer
  BufferPtr newBuffer2(new Buffer);
  newBuffer1->bzero();
  newBuffer2->bzero();
  BufferVector buffersToWrite;  // 用来和前端线程的buffers_进行swap
  buffersToWrite.reserve(16);
  while (running_) {
    assert(newBuffer1 && newBuffer1->length() == 0);
    assert(newBuffer2 && newBuffer2->length() == 0);
    assert(buffersToWrite.empty());

    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (buffers_.empty()) {
        // 没有要写入的文件，就等待指定的时间
        condition_.wait_for(lock, std::chrono::seconds(flushInterval_));
      }
      // cond_被唤醒，超时或者前端写满了一个buffer
      buffers_.push_back(std::move(currentBuffer_));  // 将当前buffer放入列表
      currentBuffer_ = std::move(newBuffer1);         // 将空闲的newBuffer1移为当前缓冲
      buffersToWrite.swap(buffers_);                  // 交换数据，并置空buffers
      if (!nextBuffer_) {
        // 如果下一个指针为空，那么将newBuffer2移过去
        nextBuffer_ = std::move(newBuffer2);
      }
    }

    assert(!buffersToWrite.empty());  // 确保写入非空

    if (buffersToWrite.size() > 25) {
      // 如果有个数大于25，此时发生了消息堆积，删除多余数据
      char buf[256];
      snprintf(buf, sizeof buf,
               "Dropped log messages at %s, %zd larger buffers\n",
               Timestamp::now().toFormattedString().c_str(),
               buffersToWrite.size() - 2);
      fputs(buf, stderr);
      output.append(buf, static_cast<int>(strlen(buf)));
      // 丢掉多余日志，以腾出内存，仅保留两块缓冲区
      buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
    }

    for (const auto& buffer : buffersToWrite) {
      // FIXME: use unbuffered stdio FILE ? or use ::writev ?
      output.append(buffer->data(), buffer->length());
    }

    if (buffersToWrite.size() > 2) {
      // drop non-bzero-ed buffers, avoid trashing
      buffersToWrite.resize(2);
    }

    if (!newBuffer1) {
      assert(!buffersToWrite.empty());
      newBuffer1 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer1->reset();
    }

    if (!newBuffer2) {
      assert(!buffersToWrite.empty());
      newBuffer2 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer2->reset();
    }

    buffersToWrite.clear();
    output.flush();
  }
  output.flush();
}