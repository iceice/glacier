#ifndef GLACIER_HTTPDATA_
#define GLACIER_HTTPDATA_

#include <unistd.h>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

class EventLoop;
class Channel;
class Timer;

// 三种连接状态：已连接，正在断开连接，已断开
enum ConnectionState { CONNECTED = 0, DISCONNECTING, DISCONNECTED };

// HTTP三种请求方式：GET，POST，HEAD
enum HttpMethod { POST = 1, GET, HEAD };

// HTTP1.0 or HTTP1.1
enum HttpVersion { HTTP_10 = 1, HTTP_11 };

// 当前处理阶段
enum ProcessState { PARSE_URI = 1, PARSE_HEADERS, RECV_BODY, ANALYSIS, FINISH };

enum ParseState {
  START = 0,
  KEY,
  COLON,
  SPACES_AFTER_COLON,
  VALUE,
  CR,
  LF,
  END_CR,
  END_LF
};

enum URIState {
  PARSE_URI_AGAIN = 1,
  PARSE_URI_ERROR,
  PARSE_URI_SUCCESS,
};

enum HeaderState {
  PARSE_HEADER_SUCCESS = 1,
  PARSE_HEADER_AGAIN,
  PARSE_HEADER_ERROR
};

enum AnalysisState { ANALYSIS_SUCCESS = 1, ANALYSIS_ERROR };

/*
 * HttpData
 *
 * 包含一个HTTP连接请求的相关信息，通过一个timer来进行超时关闭处理
 * 当我们accept一个新的连接的时候，会分配一个connfd文件描述符，通过
 * 这个文件描述符来进行通信，所以每个HttpData对应一个connfd
 */
class HttpData : public std::enable_shared_from_this<HttpData> {
 public:
  typedef std::shared_ptr<Channel> ChannelPtr;

  HttpData(EventLoop* loop, int connfd);

  ~HttpData() { ::close(fd_); }

  void reset();

  void seperateTimer();

  void linkTimer(std::shared_ptr<Timer> t) { timer_ = t; }

  ChannelPtr getChannel() { return channel_; }

  EventLoop* getLoop() { return loop_; }

  void handleClose();

  void newEvent();

 private:
  EventLoop* loop_;        // 对应的事件
  ChannelPtr channel_;     // 对应的channel
  int fd_;                 // 对应的文件描述符connfd
  bool error_;             // 是否出错
  std::string inBuffer_;   // 输入缓冲
  std::string outBuffer_;  // 输出缓冲

  ConnectionState connstate_;  // 连接状态
  HttpMethod method_;          // 请求方式
  HttpVersion version_;        // 版本
  std::string filename_;       // 文件名字
  std::string path_;           // 文件路径
  size_t readpos_;             // 当前读取的下标
  bool keepAlive_;             // 是否是长连接

  ProcessState state_;
  ParseState parse_state_;

  std::map<std::string, std::string> headers_;
  std::weak_ptr<Timer> timer_;

  void handleRead();
  void handleWrite();
  void handleConn();
  void handleError(int fd, int errnum, std::string short_msg);

  URIState parseURI();
  HeaderState parseHeaders();
  AnalysisState analysisRequest();
};

class MimeType {
 private:
  static void init();
  static std::unordered_map<std::string, std::string> mime;
  MimeType();
  MimeType(const MimeType& m);

 public:
  static std::string getMime(const std::string& suffix);

 private:
  static pthread_once_t once_control;
};

#endif  // GLACIER_HTTPDATA_