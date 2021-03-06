#include "glacier/channel.h"
#include <sys/epoll.h>
#include "glacier/base/logging.h"

Channel::Channel(EventLoop *loop)
    : loop_(loop), fd_(0), events_(0), revents_(0), lastevents_(0) {}
Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), lastevents_(0) {}

Channel::~Channel() {}

void Channel::handleEvents() {
  events_ = 0;
  if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
    // 文件描述符被挂断，并且文件描述符不可读
    return;
  }
  if (revents_ & EPOLLERR) {
    // 文件描述符发生错误
    try {
      errorCallback_();
    } catch (const std::bad_function_call &e) {
      LOG_ERROR << "errorCallback_";
    }
    return;
  }
  if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    // 文件描述符可读，或有紧急的数据可读，或对端断开连接
    try {
      readCallback_();
    } catch (const std::bad_function_call &e) {
      LOG_ERROR << "errorCallback_";
    }
  }
  if (revents_ & EPOLLOUT) {
    // 文件描述符可写
    try {
      writeCallback_();
    } catch (const std::bad_function_call &e) {
      LOG_ERROR << "errorCallback_";
    }
  }
  // 更新events
  try {
    connCallback_();
  } catch (const std::bad_function_call &e) {
    LOG_ERROR << "errorCallback_";
  }
}