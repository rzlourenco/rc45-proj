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

static char CS_name[] = "localhost";
static int CS_tcp_socket = -1;
static struct sockaddr_in CS_addr;
static int CS_port = 58000 + NG;
static int SS_tcp_socket = -1;
static int SS_port = 59000;
static struct sockaddr_in SS_addr;

void tcp_loop(void);

int main(int argc, char *argv[]) {
  // Make sure child processes are reaped by the kernel
  signal(SIGCHLD, SIG_IGN);

  parse_args(argc, argv, &SS_port, NULL);

  CS_tcp_socket = connect_tcp(CS_name, CS_port, &CS_addr);
  if (CS_tcp_socket == -1) {
    E("Could not connect to central server (%s)", strerror(errno));
  }

  SS_tcp_socket = setup_tcp_server(SS_port, &SS_addr);

  return 0;
}

void tcp_loop()
{
  for (;;)
  {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int fd = accept(SS_tcp_socket, (struct sockaddr *)&client_addr, &client_len);
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
