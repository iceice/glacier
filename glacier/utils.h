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

/* Simplifies calls to bind(), connect(), and accept() */
typedef struct sockaddr SA;

/* Persistent state for the robust I/O (Rio) package */
#define RIO_BUFSIZE 8192
typedef struct {
  int rio_fd;                /* Descriptor for this internal buf */
  int rio_cnt;               /* Unread bytes in internal buf */
  char *rio_bufptr;          /* Next unread byte in internal buf */
  char rio_buf[RIO_BUFSIZE]; /* Internal buffer */
} rio_t;

/* Misc constants */
#define MAXLINE 8192 /* Max text line length */
#define MAXBUF 8192  /* Max I/O buffer size */
#define LISTENQ 1024 /* Second argument to listen() */

/* Reentrant protocol-independent client/server helpers */
int open_clientfd(char *hostname, char *port);
int open_listenfd(const char *port);
int setSocketNonBlocking(int fd);
void setSocketNodelay(int fd);

#endif  // GLACIER_UTILS_