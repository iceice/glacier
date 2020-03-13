#ifndef GLACIER_UTILS_
#define GLACIER_UTILS_

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <string>

/* Simplifies calls to bind(), connect(), and accept() */
typedef struct sockaddr SA;

/* Reentrant protocol-independent client/server helpers */
int open_clientfd(char *hostname, char *port);
int open_listenfd(const char *port);
int setSocketNonBlocking(int fd);
void setSocketNodelay(int fd);

ssize_t readn(int fd, std::string &buf, bool &zero);
ssize_t writen(int fd, std::string &buf);
ssize_t writen(int fd, void *buf, size_t n);

#endif  // GLACIER_UTILS_