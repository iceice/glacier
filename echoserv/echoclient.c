/*

  ECHOCLIENT.C
  ==========
  (c) Yansong Li, 2020
  Email: liyansong.cs@gmail.com
  
  Simple TCP/IP echo client.

*/


#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "helper.h"           /*  our own helper functions  */

/*  Global constants  */
#define ECHO_PORT          (2020)
#define MAX_LINE           (1000)


char *Fgets(char *ptr, int n, FILE *stream) 
{
    char *rptr;

    if (((rptr = fgets(ptr, n, stream)) == NULL) && ferror(stream))

    return rptr;
}

int main(int argc, char *argv[])
{
    int       clientfd;              /*  client socket             */
    char     *buffer[MAX_LINE];      /*  character buffer          */
    char     *endptr;                /*  for strtol()              */
    short int port;                  /*  port number               */
    // struct    in_addr     host;      /*  host address              */
    struct    sockaddr_in servaddr;  /*  socket address structure  */

    /* Get host address and port from the command line */
    if (argc == 3)
    {
        // change network to int
        // if (inet_aton("127.0.0.1", &host) != 1)
        if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) < 0)
        {
            fprintf(stderr, "ECHOCLIENT: Invalid host address.\n");
            exit(EXIT_FAILURE);
        }
        // change string port to int
        port = strtol(argv[2], &endptr, 0);
        if (*endptr)
        {
            fprintf(stderr, "ECHOCLIENT: Invalid port number.\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        fprintf(stderr, "ECHOCLIENT: usage: %s <host> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Create the client socket */
    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "ECHOCLIENT: Error creating listening socket.\n");
        exit(EXIT_FAILURE);
    }

    /*  Set all bytes in socket address structure to
        zero, and fill in the relevant data members   */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(port);

    // /* Connect our socket address to the client socket */
    if (connect(clientfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        fprintf(stderr, "ECHOCLIENT: Error calling connect()\n");
        exit(EXIT_FAILURE);   
    }

    while (fgets(buffer, MAX_LINE, stdin) != NULL)
    {
        /*  Retrieve an input line from the connected socket
            then simply write it back to the same socket.     */
        Writeline(clientfd, buffer, strlen(buffer));
        Readline(clientfd, buffer, MAX_LINE-1);
        fputs(buffer, stdout);
    }

    close(clientfd);
    
    exit(0);
}