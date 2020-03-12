#ifndef GLACIER_CHANNEL_
#define GLACIER_CHANNEL_

#include <memory>
#include "glacier/base/uncopyable.h"
#include "glacier/eventloop.h"

class EventLoop;

/*
 * Channel 事件分发器
 *
 * 每个channel对象自始至终只属于一个EventLoop，即只属于一个IO线程
 * 它的作用是负责一个文件描述符fd的IO事件分发，但它并不拥有这个fd，
 * 也不会再析构的时候关闭这个fd，只是把不同的IO事件分发为不同的回调函数
 */
class Channel : Uncopyable {
 public:
  typedef std::shared_ptr<Channel> ptr;
  typedef std::function<void()> EventCallback;

  Channel(EventLoop *loop);
  Channel(EventLoop *loop, int fd);
  ~Channel();

  int getFd() { return fd_; }
  void setFd(int fd) { fd_ = fd; }

  void handleEvents();
  void setReadCallback(EventCallback cb) { readCallback_ = std::move(cb); }
  void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
  void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }
  void setConnCallback(EventCallback cb) { connCallback_ = std::move(cb); }

  void setEvents(uint32_t ev) { events_ = ev; }
  void setRevents(uint32_t rev) { revents_ = rev; }

  uint32_t &getEvents() { return events_; }
  uint32_t getLastEvents() { return lastevents_; }

  bool EqualAndUpdateLastEvents() {
    bool flag = (lastevents_ == events_);
    lastevents_ = events_;
    return flag;
  }

 private:
  EventLoop *loop_;      // channel所属的loop
  int fd_;               // channel负责的文件描述符
  uint32_t events_;      // 注册的事件
  uint32_t revents_;     // epoll设置的就绪的事件
  uint32_t lastevents_;  // 上一个事件的状态

  EventCallback readCallback_;   // 读事件回调
  EventCallback writeCallback_;  // 写事件回调
  EventCallback errorCallback_;  // 错误事件回调
  EventCallback connCallback_;   // 连接事件回调
};

#endif  // GLACIER_CHANNEL_