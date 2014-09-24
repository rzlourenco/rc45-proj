#include "common.h"

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static char CS_name[128];
static int CS_tcp_socket = INVALID_SOCKET;
// static int CS_udp_socket = INVALID_SOCKET;
static int CS_port = 58000 + NG;
static struct sockaddr_in CS_addr;
static struct sockaddr *CS_addr_ptr = NULL;

void setup_tcp(void);
void setup_udp(void);
void tcp_loop(void);
void udp_loop(void);

int main(int argc, char *argv[]) {
  parse_args(argc, argv, &CS_port, CS_name);

  CS_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  CS_addr.sin_family = AF_INET;
  CS_addr.sin_port = htons(CS_port);

  pid_t udp_server = fork();
  if (udp_server == 0) {
    setup_udp();
    udp_loop();
  }
  else if (udp_server == -1) {
    fprintf(stderr, "Could not fork UDP server: %s\n", strerror(errno));
    exit(1);
  }
  else {
    // bind
    // listen
    tcp_loop();
  }

  return 0;
}

void setup_tcp() {
  CS_tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (CS_tcp_socket == -1) {
    HANDLE_ERRNO("Could not create TCP socket");
  }

  if (bind(CS_tcp_socket, CS_addr_ptr, sizeof(CS_addr)) == -1) {
    HANDLE_ERRNO("Could not bind TCP socket");
  }

  if (listen(CS_tcp_socket, 8) == -1) {
    HANDLE_ERRNO("Could not listen on TCP socket");
  }
}

void setup_udp() {}

void tcp_loop() {
  for (;;) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int fd = accept(CS_tcp_socket, (struct sockaddr *)&client_addr, &client_len);

    if (fd == -1) {
      HANDLE_ERRNO("Failed to accept connection");
    }

    pid_t pid = fork();
    if (pid == -1) {
      HANDLE_ERRNO("Could not fork server");
    }

    // Leave client handling to child process
    if (pid != 0) {
      close(fd);
      continue;
    }

    // Handle client requests
  }
}

void udp_loop() {}

