#ifndef GLACIER_TCPSERVER_
#define GLACIER_TCPSERVER_

#include "glacier/eventloop.h"

/*
 * TcpServer 服务器
 *
 * 使用一个accepChannel来进行accept事件的管理，accepChannel对应了mainloop
 * 以及listenfd，使用一个线程池用来进行IO任务的分发。
 *
 */
class TcpServer {
 public:
  TcpServer(EventLoop* loop, int threadNum, const char* port);

  ~TcpServer() {}

  EventLoop* getLoop() const { return loop_; }

  void start();

  void handleNewConn();

  void handleThisConn();

 private:
  EventLoop* loop_;   // 等价于mainloop
  int threadNum_;     // 线程数量
  const char* port_;  // 监听的端口号
  int listenfd_;      // 返回的文件描述符

  // EventLoopThreadPool::ptr eventLoopThreadPool_;  // 线程池
  Channel::ptr acceptChannel_;  // 对应mainloop

  bool started_;
};

#endif  // GLACIER_TCPSERVER_