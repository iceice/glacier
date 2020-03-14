#include "glacier/httpdata.h"
#include "glacier/base/log.h"
#include "glacier/channel.h"
#include "glacier/eventloop.h"
#include "glacier/timer.h"
#include "glacier/utils.h"

using namespace std;

pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
unordered_map<string, string> MimeType::mime;

const uint32_t DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;
const int DEFAULT_EXPIRED_TIME = 2000;              // ms
const int DEFAULT_KEEP_ALIVE_TIME = 5 * 60 * 1000;  // ms

void MimeType::init() {
  mime[".html"] = "text/html; charset=UTF-8";
  mime[".htm"] = "text/html; charset=UTF-8";
  mime[".ico"] = "image/x-icon";
  mime[".jpg"] = "image/jpeg";
  mime[".jpeg"] = "image/jpeg";
  mime[".png"] = "image/png";
  mime[".css"] = "text/css";
  mime[".js"] = "application/x-javascript";
  mime[".woff2?v=4.7.0"] = "application/font-woff2";
  mime["default"] = "text/html";
}

std::string MimeType::getMime(const std::string &suffix) {
  pthread_once(&once_control, MimeType::init);
  if (mime.find(suffix) == mime.end()) {
    return mime["default"];
  } else {
    return mime[suffix];
  }
}

HttpData::HttpData(EventLoop *loop, int connfd)
    : loop_(loop),
      channel_(new Channel(loop, connfd)),
      fd_(connfd),
      error_(false),
      inBuffer_(),
      outBuffer_(),
      connstate_(CONNECTED),
      method_(GET),
      version_(HTTP_11),
      filename_(),
      path_(),
      readpos_(0),
      keepAlive_(false),
      state_(PARSE_URI),
      parse_state_(START) {
  channel_->setReadCallback(bind(&HttpData::handleRead, this));
  channel_->setWriteCallback(bind(&HttpData::handleWrite, this));
  channel_->setConnCallback(bind(&HttpData::handleConn, this));
}

void HttpData::reset() {
  filename_.clear();
  path_.clear();
  readpos_ = 0;
  state_ = PARSE_URI;
  parse_state_ = START;
  headers_.clear();
  if (timer_.lock()) {
    shared_ptr<Timer> t(timer_.lock());
    t->reset();
    timer_.reset();
  }
}

void HttpData::seperateTimer() {
  if (timer_.lock()) {
    shared_ptr<Timer> t(timer_.lock());
    t->reset();
    timer_.reset();
  }
}

void HttpData::handleRead() {
  uint32_t &event = channel_->getEvents();
  do {
    bool zero = false;
    ssize_t cnt = readn(fd_, inBuffer_, zero);
    if (connstate_ == DISCONNECTING) {
      inBuffer_.clear();
      break;
    }
    if (cnt < 0) {
      error_ = true;
      handleError(fd_, 400, "Bad Request");
      break;
    }
    if (zero) {
      // 有请求出现但是读不到数据
      connstate_ = DISCONNECTING;
      if (cnt == 0) break;
    }

    // parse uri
    if (state_ == PARSE_URI) {
      URIState flag = this->parseURI();
      if (flag == PARSE_URI_AGAIN) break;
      if (flag == PARSE_URI_ERROR) {
        LOG_ERROR << "parse uri error";
        inBuffer_.clear();
        error_ = true;
        handleError(fd_, 400, "Bad Request");
        break;
      } else {
        state_ = PARSE_HEADERS;
      }
    }
    // parse headers
    if (state_ == PARSE_HEADERS) {
      HeaderState flag = this->parseHeaders();
      if (flag == PARSE_HEADER_AGAIN) break;
      if (flag == PARSE_HEADER_ERROR) {
        LOG_ERROR << "parse headers error";
        error_ = true;
        handleError(fd_, 400, "Bad Request");
        break;
      }
      if (method_ == POST)
        state_ = RECV_BODY;
      else
        state_ = ANALYSIS;
    }
    // TODO: post
    if (state_ == ANALYSIS) {
      AnalysisState flag = this->analysisRequest();
      if (flag == ANALYSIS_SUCCESS) {
        state_ = FINISH;
        break;
      } else {
        error_ = true;
        break;
      }
    }
  } while (false);

  if (!error_) {
    // 如果没有出错，判断是否需要写入数据
    if (outBuffer_.size()) handleWrite();
    // 写入过程可能出粗，所以需要再次判断
    if (!error_ && state_ == FINISH) {
      this->reset();
      if (inBuffer_.size() > 0 && connstate_ != DISCONNECTING) {
        handleRead();
      }
    } else if (!error_ && connstate_ == DISCONNECTED) {
      event |= EPOLLIN;
    }
  }
}

void HttpData::handleWrite() {
  if (!error_ && connstate_ != DISCONNECTED) {
    uint32_t &event = channel_->getEvents();
    if (writen(fd_, outBuffer_) < 0) {
      LOG_ERROR << "write error";
      event = 0;
      error_ = true;
    }
    if (outBuffer_.size()) event |= EPOLLOUT;
  }
}

void HttpData::handleConn() {
  // 当我们处理这次连接的时候，需要更新计时器
  // 所以先把计时器与这次HttpData分离
  seperateTimer();
  uint32_t &event = channel_->getEvents();
  if (!error_ && connstate_ == CONNECTED) {
    if (event != 0) {
      int t = (keepAlive_) ? DEFAULT_KEEP_ALIVE_TIME : DEFAULT_EXPIRED_TIME;
      if ((event & EPOLLIN) && (event & EPOLLOUT)) {
        event = 0;
        event |= EPOLLOUT;
      }
      event |= EPOLLET;
      loop_->updatePoller(channel_, t);
    } else if (keepAlive_) {
      event |= (EPOLLIN | EPOLLET);
      loop_->updatePoller(channel_, DEFAULT_KEEP_ALIVE_TIME);
    } else {
      event |= (EPOLLIN | EPOLLET);
      loop_->updatePoller(channel_, (DEFAULT_KEEP_ALIVE_TIME >> 1));
    }
  } else if (!error_ && connstate_ == DISCONNECTING && (event & EPOLLOUT)) {
    event = (EPOLLOUT | EPOLLET);
  } else {
    loop_->runInLoop(bind(&HttpData::handleClose, shared_from_this()));
  }
}

void HttpData::handleClose() {
  connstate_ = DISCONNECTED;
  shared_ptr<HttpData> guard(shared_from_this());
  loop_->removeFronPoller(channel_);
}

void HttpData::handleError(int fd, int err_num, string short_msg) {
  short_msg = " " + short_msg;
  char send_buff[4096];
  string body_buff, header_buff;
  body_buff += "<html><title>error</title>";
  body_buff += "<body bgcolor=\"ffffff\">";
  body_buff += to_string(err_num) + short_msg;
  body_buff += "<hr><em> LiYansong's Web Server</em>\n</body></html>";

  header_buff += "HTTP/1.1 " + to_string(err_num) + short_msg + "\r\n";
  header_buff += "Content-Type: text/html\r\n";
  header_buff += "Connection: Close\r\n";
  header_buff += "Content-Length: " + to_string(body_buff.size()) + "\r\n";
  header_buff += "Server: LiYansong's Web Server\r\n";
  ;
  header_buff += "\r\n";
  sprintf(send_buff, "%s", header_buff.c_str());
  writen(fd, send_buff, strlen(send_buff));
  sprintf(send_buff, "%s", body_buff.c_str());
  writen(fd, send_buff, strlen(send_buff));
}

void HttpData::newEvent() {
  channel_->setEvents(DEFAULT_EVENT);
  loop_->addToPoller(channel_, DEFAULT_EXPIRED_TIME);
}

URIState HttpData::parseURI() {
  string &str = inBuffer_;
  string cop = str;
  // 读到完整的请求行再开始解析请求
  size_t pos = str.find('\r', readpos_);
  if (pos == string::npos) {
    return PARSE_URI_AGAIN;
  }
  string request_line = str.substr(0, pos);
  if (str.size() > pos + 1)
    str = str.substr(pos + 1);
  else
    str.clear();
  size_t posGet = request_line.find("GET");
  size_t posPost = request_line.find("POST");
  size_t posHead = request_line.find("HEAD");

  if (posGet != string::npos) {
    pos = posGet;
    method_ = GET;
  } else if (posPost != string::npos) {
    pos = posPost;
    method_ = POST;
  } else if (posHead != string::npos) {
    pos = posHead;
    method_ = HEAD;
  } else {
    return PARSE_URI_ERROR;
  }

  pos = request_line.find("/", pos);
  if (pos == string::npos) {
    filename_ = "index.html";
    version_ = HTTP_11;
    return PARSE_URI_SUCCESS;
  }
  size_t _pos = request_line.find(' ', pos);
  if (_pos != string::npos) {
    if (_pos - pos > 1) {
      filename_ = request_line.substr(pos + 1, _pos - pos - 1);
      size_t __pos = filename_.find('?');
      if (__pos != string::npos) filename_ = filename_.substr(0, __pos);
    } else {
      filename_ = "index.html";
    }
  } else {
    return PARSE_URI_ERROR;
  }
  pos = _pos;

  // http version
  pos = request_line.find("/", pos);
  if (pos == string::npos) return PARSE_URI_ERROR;
  if (request_line.size() - pos <= 3) return PARSE_URI_ERROR;
  string v = request_line.substr(pos + 1, 3);
  if (v == "1.0")
    version_ = HTTP_10;
  else if (v == "1.1")
    version_ = HTTP_11;
  else
    return PARSE_URI_ERROR;

  return PARSE_URI_SUCCESS;
}

HeaderState HttpData::parseHeaders() {
  string &str = inBuffer_;
  size_t key_start = 0, key_end = 0, value_start = 0, value_end = 0;
  size_t now_read_line_begin = 0;
  bool finish = false;
  size_t i = 0;
  for (; i < str.size() && !finish; ++i) {
    switch (parse_state_) {
      case START: {
        if (str[i] == '\n' || str[i] == '\r') break;
        parse_state_ = KEY;
        key_start = i;
        now_read_line_begin = i;
        break;
      }
      case KEY: {
        if (str[i] == ':') {
          key_end = i;
          if (key_end - key_start <= 0) return PARSE_HEADER_ERROR;
          parse_state_ = COLON;
        } else if (str[i] == '\n' || str[i] == '\r')
          return PARSE_HEADER_ERROR;
        break;
      }
      case COLON: {
        if (str[i] == ' ') {
          parse_state_ = SPACES_AFTER_COLON;
        } else
          return PARSE_HEADER_ERROR;
        break;
      }
      case SPACES_AFTER_COLON: {
        parse_state_ = VALUE;
        value_start = i;
        break;
      }
      case VALUE: {
        if (str[i] == '\r') {
          parse_state_ = CR;
          value_end = i;
          if (value_end - value_start <= 0) return PARSE_HEADER_ERROR;
        } else if (i - value_start > 255)
          return PARSE_HEADER_ERROR;
        break;
      }
      case CR: {
        if (str[i] == '\n') {
          parse_state_ = LF;
          string key(str.begin() + key_start, str.begin() + key_end);
          string value(str.begin() + value_start, str.begin() + value_end);
          headers_[key] = value;
          now_read_line_begin = i;
        } else
          return PARSE_HEADER_ERROR;
        break;
      }
      case LF: {
        if (str[i] == '\r') {
          parse_state_ = END_CR;
        } else {
          key_start = i;
          parse_state_ = KEY;
        }
        break;
      }
      case END_CR: {
        if (str[i] == '\n') {
          parse_state_ = END_LF;
        } else
          return PARSE_HEADER_ERROR;
        break;
      }
      case END_LF: {
        finish = true;
        key_start = i;
        now_read_line_begin = i;
        break;
      }
    }
  }
  if (parse_state_ == END_LF) {
    str = str.substr(i);
    return PARSE_HEADER_SUCCESS;
  }
  str = str.substr(now_read_line_begin);
  return PARSE_HEADER_AGAIN;
}

AnalysisState HttpData::analysisRequest() {
  // handleError(fd_, 404, "Not Found!");
  // return ANALYSIS_ERROR;
  
  if (method_ == GET) {
    string header;
    header += "HTTP/1.1 200 OK\r\n";
    if (headers_.find("Connection") != headers_.end() &&
        (headers_["Connection"] == "Keep-Alive" ||
         headers_["Connection"] == "keep-alive")) {
      keepAlive_ = true;
      header += string("Connection: Keep-Alive\r\n") +
                "Keep-Alive: timeout=" + to_string(DEFAULT_KEEP_ALIVE_TIME) +
                "\r\n";
    }
    // 1.get file type
    string filetype;
    size_t pos = filename_.find('.');
    if (pos == string::npos)
      filetype = MimeType::getMime("default");
    else
      filetype = MimeType::getMime(filename_.substr(pos));

    // 2.read file
    struct stat sbuf;
    if (stat(filename_.c_str(), &sbuf) < 0) {
      header.clear();
      handleError(fd_, 404, "Not Found!");
      return ANALYSIS_ERROR;
    }
    header += "Content-Type: " + filetype + "\r\n";
    header += "Content-Length: " + to_string(sbuf.st_size) + "\r\n";
    header += "Server: LiYansong's Web Server\r\n";
    // 头部结束
    header += "\r\n";
    // 放入缓冲
    outBuffer_ += header;
    // 开始读取文件
    int srcfd = open(filename_.c_str(), O_RDONLY, 0);
    if (srcfd < 0) {
      outBuffer_.clear();
      handleError(fd_, 404, "Not Found!");
      return ANALYSIS_ERROR;
    }
    char *srcp = static_cast<char *>(
        mmap(0, sbuf.st_size, PROT_READ, MAP_PRIVATE, srcfd, 0));
    ::close(srcfd);
    outBuffer_ += string(srcp, srcp + sbuf.st_size);
    munmap(srcp, sbuf.st_size);
    return ANALYSIS_SUCCESS;
  }
  return ANALYSIS_ERROR;
}