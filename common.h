#ifndef COMMON_H
#define COMMON_H
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>

#define NG 10
#define INVALID_SOCKET (-1)
#define HANDLE_ERRNO(s) { fprintf(stderr, s ":%s\n", strerror(errno)); exit(1); }

char *receive_udp(int fd, struct sockaddr *addr);
char *receive_tcp(int fd, struct sockaddr *addr);
void parse_args(int argc, char *argv[], int *port, char *name);

#endif // COMMON_H

