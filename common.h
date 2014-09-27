#ifndef COMMON_H
#define COMMON_H
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>

#define NG 10
#define MAX_FILE_SIZE 4096

#ifdef NDEBUG
#define DEBUG 0
#else
#define DEBUG 1
#endif

#define D(fmt, ...) do { if (DEBUG) { printf("[DEBUG] " fmt "\n", ##__VA_ARGS__); } } while (0)
#define W(fmt, ...) do { fprintf(stderr, "Warning: " fmt "\n", ##__VA_ARGS__); } while (0)
#define E(fmt, ...) do { fprintf(stderr, "Error: " fmt "\n", ##__VA_ARGS__); exit(1); } while (0)

int setup_tcp_server(in_port_t port, struct sockaddr_in *outaddr);
int setup_udp_server(in_port_t port, struct sockaddr_in *outaddr);
int connect_tcp(const char *hostname, in_port_t port, struct sockaddr_in *outaddr);
int connect_udp(const char *hostname, in_port_t port, struct sockaddr_in *outaddr);
void parse_args(int argc, char *argv[], int *port, char *name);

#endif // COMMON_H

