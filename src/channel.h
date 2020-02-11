#ifndef SRC_CHANNEL_
#define SRC_CHANNEL_

#include "base/uncopyable.h"
#include "base/timestamp.h"

#include <functional>
#include <memory>

class EventLoop;

//
// Channel 事件分发器
//
// 每个channel对象自始至终只属于一个EventLoop，即只属于一个IO线程
// 它的作用是负责一个文件描述符fd(eventfd, timerfd, signalfd)
// 的IO事件分发，但它并不拥有这个fd，也不会再析构的时候关闭这个fd，
// 只是把不同的IO事件分发为不同的回调函数
class Channel : Uncopyable
{
public:
    typedef std::function<void()> EventCallback;              // 事件回调函数对象类型
    typedef std::function<void(Timestamp)> ReadEventCallback; // 读事件回调函数对象类型

    Channel(EventLoop *loop, int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    // 将channel绑定到由shared_ptr管理的对象上
    void tie(const std::shared_ptr<void> &);

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }
    bool isNoneEvent() const { return events_ == kNoneEvent; }

    void enableReading()
    {
        events_ |= kReadEvent;
        update();
    }
    void disableReading()
    {
        events_ &= ~kReadEvent;
        update();
    }

    void enableWriting()
    {
        events_ |= kWriteEvent;
        update();
    }
    void disableWriting()
    {
        events_ &= ~kWriteEvent;
        update();
    }

    void disableAll()
    {
        events_ = kNoneEvent;
        update();
    }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    // for Poller
    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    // for debug
    string reventsToString() const;
    string eventsToString() const;

    void doNotLogHup() { logHup_ = false; }

    EventLoop *ownerLoop() { return loop_; }
    void remove();

private:
    static string eventsToString(int fd, int ev);

    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;  //无事件
    static const int kReadEvent;  //可读事件
    static const int kWriteEvent; //可写事件

    EventLoop *loop_; // channel所属的loop
    const int fd_;    // channel负责的文件描述符
    int events_;      // 注册的事件
    int revents_;     // poller设置的就绪的事件
    int index_;       // poller使用的下标
    bool logHup_;     // 是否生成某些日志

    // 防止handleEvent()运行期间其owner对象析构，导致Channel本身被销毁。
    std::weak_ptr<void> tie_; 
    bool tied_;
    
    bool eventHandling_; // 是否正在处理事件
    bool addedToLoop_;

    ReadEventCallback readCallback_; // 读事件回调
    EventCallback writeCallback_;    // 写事件回调
    EventCallback closeCallback_;    // 关闭事件回调
    EventCallback errorCallback_;    // 错误事件回调
};

#endif // SRC_CHANNEL_
