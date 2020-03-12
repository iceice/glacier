#ifndef GLACIER_HTTPSERVER_
#define GLACIER_HTTPSERVER_

#include <unistd.h>
#include <memory>
#include "glacier/channel.h"
#include "glacier/eventloop.h"
#include "glacier/utils.h"

class EventLoop;
class Channel;

class HttpServer : public std::enable_shared_from_this<HttpServer> {
 public:
  HttpServer(EventLoop* loop, int connfd);
  ~HttpServer() { ::close(fd_); }

  void reset();

  std::shared_ptr<Channel> getChannel() { return channel_; }
  EventLoop* getLoop() { return loop_; }

  void handleRead();
  void handleWrite();
  void handleConn();
  void handleClose();

  void newEvent();

 private:
  EventLoop* loop_;
  std::shared_ptr<Channel> channel_;
  int fd_;
  char inBuffer_[MAXLINE];
  char outBuffer_[MAXLINE];
};

#endif  // GLACIER_HTTPSERVER_