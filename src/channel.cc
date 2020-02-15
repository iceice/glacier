#include "src/channel.h"

#include "src/epoll.h"
#include "src/event_loop.h"
#include "src/utils.h"

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), lastevents_(0) {}

Channel::~Channel() {}

void Channel::handleEvents()
{
    events_ = 0;
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        // 对应的文件描述符被挂断，并且不可读
        events_ = 0;
        return;
    }
    if (revents_ & EPOLLERR)
    {
        // 对应的文件描述符发生错误
        if (errorCallback_)
            errorCallback_();
        events_ = 0;
        return;
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        // 对应的文件描述符可以读 | 有紧急的数据可读 | 文件描述符被挂断
        if (readCallback_)
            readCallback_();
    }
    if (revents_ & EPOLLOUT)
    {
        // 对应的文件描述符可以写
        if (writeCallback_)
            writeCallback_();
    }
    // 最后执行一遍连接处理
    if (connCallback_)
        connCallback_();
}

bool Channel::EqualAndUpdateLastEvents()
{
    bool flag = (lastevents_ == events_);
    lastevents_ = events_;
    return flag;
}