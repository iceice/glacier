#include "glacier/httpserver.h"
#include "glacier/base/log.h"

const uint32_t DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;
const int DEFAULT_EXPIRED_TIME = 2000;  // ms

HttpServer::HttpServer(EventLoop* loop, int connfd)
    : loop_(loop), channel_(new Channel(loop, connfd)), fd_(connfd) {
  // 设置connfd对应的channel的回调函数
  channel_->setReadCallback(std::bind(&HttpServer::handleRead, this));
  channel_->setWriteCallback(std::bind(&HttpServer::handleWrite, this));
  channel_->setConnCallback(std::bind(&HttpServer::handleConn, this));
}

void HttpServer::newEvent() {
  channel_->setEvents(DEFAULT_EVENT);
  loop_->addToPoller(channel_, DEFAULT_EXPIRED_TIME);
}

void HttpServer::handleRead() { LOG_INFO << "HttpServer handle read"; }

void HttpServer::handleWrite() {}

void HttpServer::handleConn() {}

void HttpServer::handleClose() {}