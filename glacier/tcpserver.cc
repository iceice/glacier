#include "glacier/tcpserver.h"
#include "glacier/base/log.h"
#include "glacier/utils.h"

TcpServer::TcpServer(EventLoop* loop, int threadNum, const char* port)
    : loop_(loop),
      threadNum_(threadNum),
      port_(port),
      listenfd_(open_listenfd(port)),
      // eventLoopThreadPool_(new EventLoopThreadPool(loop, threadNum)),
      acceptChannel_(new Channel(loop)),
      started_(false) {
  acceptChannel_->setFd(listenfd_);
  // TODO setSocketNonBlocking
}

void TcpServer::start() {
  acceptChannel_->setEvents(EPOLLIN | EPOLLET);
  acceptChannel_->setReadCallback(std::bind(&TcpServer::handleNewConn, this));
  acceptChannel_->setConnCallback(std::bind(&TcpServer::handleThisConn, this));
  loop_->addToPoller(acceptChannel_, 0);
  started_ = true;
}

void TcpServer::handleNewConn() {
  int connfd;          /*  connecting socket   */
  char hostname[8192]; /*  host name           */
  char port[8192];     /*  port number         */

  struct sockaddr_storage clientaddr;
  socklen_t clientlen = sizeof(clientaddr);

  while ((connfd = accept(listenfd_, reinterpret_cast<SA*>(&clientaddr),
                          &clientlen)) > 0) {
    getnameinfo(reinterpret_cast<SA*>(&clientaddr), clientlen, hostname, 8192,
                port, 8192, 0);
    LOG_INFO << "New connection from " << hostname << " : " << port;
    // 将connection fd设为非阻塞
    if (setSocketNonBlocking(connfd) < 0) {
      LOG_ERROR << "Set non block failed";
      return;
    } else {
      LOG_INFO << "set " << connfd << " non block success";
    }
    setSocketNodelay(connfd);
    // TODO 线程池
  }
}

void TcpServer::handleThisConn() {
  loop_->updatePoller(acceptChannel_);
}