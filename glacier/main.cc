#include "glacier/base/log.h"
#include "glacier/eventloop.h"
#include "glacier/tcpserver.h"



using namespace std;
using namespace glacier;

const size_t k_roollsize = 1000 * 1000 * 1000;

#define MAXLINE 8192 /* Max text line length */

int main(int argc, const char *argv[]) {
  LOG_WARN << "======== Glacier started ========";
  const char *port = argv[1];
  EventLoop mainloop;
  TcpServer server(&mainloop, 4, port);
  server.start();
  mainloop.loop();
  return 0;
}
