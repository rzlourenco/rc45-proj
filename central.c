#include "common.h"

#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int CS_tcp_socket = -1;
static int CS_udp_socket = -1;
static int CS_port = 58000 + NG;
static struct sockaddr_in CS_addr;

void tcp_loop(void);
void udp_loop(void);

int main(int argc, char *argv[]) {
  // Make sure child processes are reaped by the kernel
  signal(SIGCHLD, SIG_IGN);

  parse_args(argc, argv, &CS_port, NULL);

  pid_t udp_server = fork();
  if (udp_server == 0) {
    CS_udp_socket = setup_udp_server(CS_port, &CS_addr);
    if (CS_udp_socket == -1) {
      E("Could not create UDP socket (%s)", strerror(errno));
    }
    udp_loop();
  }
  else if (udp_server == -1) {
    E("Could not fork UDP server (%s)", strerror(errno));
  }
  else {
    CS_tcp_socket = setup_tcp_server(CS_port, &CS_addr);
    if (CS_tcp_socket == -1) {
      E("Could not create TCP socket (%s)", strerror(errno));
    }
    tcp_loop();
  }

  return 0;
}

void tcp_loop() {
  for (;;) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int fd = accept(CS_tcp_socket, (struct sockaddr *)&client_addr, &client_len);

    if (fd == -1) {
      E("Failed to accept TCP connection (%s)", strerror(errno));
    }

    pid_t pid = fork();
    if (pid == -1) {
      E("Could not fork TCP server (%s)", strerror(errno));
    }

    // Leave client handling to child process
    if (pid != 0) {
      close(fd);
      continue;
    }

    // Handle client requests
    
    // Terminate process
    exit(0);
  }
}

void udp_loop() {
  for (;;) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    char msg[64 * 1024] = {0};
    if (recvfrom(CS_udp_socket, msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, &client_len) == -1) {
      E("Failed to receive data from client (%s)", strerror(errno));
    }

    // TODO handle errors
    if (fork() != 0) {
      continue;
    }

    if (strncmp("LST\n", msg, 4) == 0) {
      // TODO handle errors
      static char flist[] = "AWL 127.0.0.1 59000 4 a.txt b.png c.d d.c\n";
      sendto(CS_udp_socket, flist, sizeof(flist) - 1, 0, (struct sockaddr *)&client_addr, client_len); 
    }
    else {
      static char resp[] = "ERR\n";
      sendto(CS_udp_socket, resp, sizeof(resp) - 1, 0, (struct sockaddr *)&client_addr, client_len);
    }

    exit(0);
  }
}

