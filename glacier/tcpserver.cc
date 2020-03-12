#include "glacier/tcpserver.h"
#include "glacier/base/log.h"
#include "glacier/utils.h"

const int MAXFDS = 100000;

TcpServer::TcpServer(EventLoop* loop, int threadNum, const char* port)
    : loop_(loop),
      threadNum_(threadNum),
      port_(port),
      listenfd_(open_listenfd(port)),
      eventLoopThreadPool_(new EventLoopThreadPool(loop, threadNum)),
      acceptChannel_(new Channel(loop)),
      started_(false) {
  acceptChannel_->setFd(listenfd_);
  if (setSocketNonBlocking(listenfd_) < 0) {
    LOG_ERROR << "Set listen fd non block failed";
  }
}

void TcpServer::start() {
  eventLoopThreadPool_->start();
  acceptChannel_->setEvents(EPOLLIN | EPOLLET);
  acceptChannel_->setReadCallback(std::bind(&TcpServer::handleNewConn, this));
  acceptChannel_->setConnCallback(std::bind(&TcpServer::handleThisConn, this));
  loop_->addToPoller(acceptChannel_, 0);
  started_ = true;
}

void TcpServer::handleNewConn() {
  int connfd = 0;
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  memset(&client_addr, 0, sizeof(struct sockaddr_in));
  while ((connfd = accept(listenfd_, reinterpret_cast<SA*>(&client_addr),
                          &client_addr_len)) > 0) {
    LOG_INFO << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":"
             << ntohs(client_addr.sin_port);

    // 限制服务器的最大并发连接数
    if (connfd >= MAXFDS) {
      ::close(connfd);
      continue;
    }
    // 将connection fd设为非阻塞
    if (setSocketNonBlocking(connfd) < 0) {
      LOG_ERROR << "Set non block failed";
      return;
    }

    setSocketNodelay(connfd);

    EventLoop* loop = eventLoopThreadPool_->getNextLoop();
    std::shared_ptr<HttpServer> req_info(new HttpServer(loop, connfd));
    req_info->getChannel()->setHolder(req_info);
    loop->queueInLoop(std::bind(&HttpServer::newEvent, req_info));
  }
  acceptChannel_->setEvents(EPOLLIN | EPOLLET);
}

void TcpServer::handleThisConn() { loop_->updatePoller(acceptChannel_); }