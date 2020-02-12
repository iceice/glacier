#ifndef SRC_SOCKET_
#define SRC_SOCKET_

#include "base/uncopyable.h"

struct tcp_info;   // 有关tcp的结构体，在<netinet/tcp.h>文件中
class InetAddress; // 对sockaddr_in的简单封装

/*
 * 用RAII方法封装socket的文件描述符
 */
class Socket : Uncopyable
{
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}

    ~Socket();

    int fd() const { return sockfd_; }

    bool getTcpInfo(struct tcp_info *) const;
    bool getTcpInfoString(char *buf, int len) const;

    void bindAddress(const InetAddress &localaddr);
    void listen();

    int accept(InetAddress *peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on); // 是否开启：TCP_NODELAY
    void setReuseAddr(bool on);  // 是否开启：SO_REUSEADDR
    void setReusePort(bool on);  // 是否开启：SO_REUSEPORT
    void setKeepAlive(bool on);  // 是否开启：SO_KEEPALIVE

private:
    const int sockfd_;
};

#endif // SRC_SOCKET_
