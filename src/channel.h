#ifndef SRC_CHANNEL_
#define SRC_CHANNEL_

#include "src/timer.h"

#include <sys/epoll.h>
#include <functional>
#include <unordered_map>
#include <memory>

class EventLoop;
class HttpData;

/*
 * Channel 事件分发器
 *
 * 每个channel对象自始至终只属于一个EventLoop，即只属于一个IO线程
 * 它的作用是负责一个文件描述符fd的IO事件分发，但它并不拥有这个fd，
 * 也不会再析构的时候关闭这个fd，只是把不同的IO事件分发为不同的回调函数
 */
class Channel : Uncopyable
{
public:
    typedef std::function<void()> EventCallback;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    int getFd() { return fd_; }

    std::shared_ptr<HttpData> getHolder()
    {
        std::shared_ptr<HttpData> res(holder_.lock());
        return res;
    }
    void setHolder(std::shared_ptr<HttpData> holder)
    {
        holder_ = holder;
    }

    void setReadCallback(EventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }
    void setConnCallback(EventCallback cb) { connCallback_ = std::move(cb); }

    void handleEvents();

    void setEvents(int ev) { events_ = ev; }
    uint32_t &getEvents() { return events_; }

    void setRevents(int ev) { revents_ = ev; }
    uint32_t getLastEvents() { return lastevents_; }

    bool EqualAndUpdateLastEvents();

private:
    int parseURI();
    int parseHeaders();
    int analysisRequest();

    EventLoop *loop_;                // channel所属的loop
    const int fd_;                   // channel负责的文件描述符
    uint32_t events_;                // 注册的事件
    uint32_t revents_;               // poller设置的就绪的事件
    uint32_t lastevents_;            // 上一个事件的状态
    std::weak_ptr<HttpData> holder_; // 指向上层持有该Channel的对象

    EventCallback readCallback_;  // 读事件回调
    EventCallback writeCallback_; // 写事件回调
    EventCallback errorCallback_; // 错误事件回调
    EventCallback connCallback_;  // 连接事件回调
};

typedef std::shared_ptr<Channel> SP_Channel;

#endif // SRC_CHANNEL_
