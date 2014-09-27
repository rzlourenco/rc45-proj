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

static char CS_name[128];
static int CS_tcp_socket = INVALID_SOCKET;
static int CS_udp_socket = INVALID_SOCKET;
static int CS_port = 58000 + NG;
static struct sockaddr_in CS_addr;
static struct sockaddr *CS_addr_ptr = (struct sockaddr *)&CS_addr; // tas a castar um tipo de ponteiro a outro tipo

void setup_tcp(void);
void setup_udp(void);
void tcp_loop(void);
void udp_loop(void);

int main(int argc, char *argv[]) {
  // Make sure child processes are reaped by the kernel
  signal(SIGCHLD, SIG_IGN);

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
    setup_tcp();
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

void setup_udp() {
  CS_udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (CS_udp_socket == -1) {
    HANDLE_ERRNO("Could not create UDP socket");
  }

  if (bind(CS_udp_socket, CS_addr_ptr, sizeof(CS_addr)) == -1) {
    HANDLE_ERRNO("Could not bind UDP socket");
  }
}

void tcp_loop() {
  for (;;) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int fd = accept(CS_tcp_socket, (struct sockaddr *)&client_addr, &client_len);

    if (fd == -1) {
      HANDLE_ERRNO("Failed to accept TCP connection");
    }

    pid_t pid = fork();
    if (pid == -1) {
      HANDLE_ERRNO("Could not fork TCP server");
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

    char *msg = calloc(1025, sizeof(char));
    assert(msg != NULL);

    if (recvfrom(CS_udp_socket, msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, &client_len) == -1) {
      HANDLE_ERRNO("Failed to receive data from client");
    }

    // TODO handle errors
    if (fork() != 0) {
      free(msg);
      continue;
    }

    if (strncmp("LST\n", msg, 4) == 0) {
      // TODO handle errors
      static char msg[] = "AWL 127.0.0.1 59000 4 a.txt b.png c.d d.c\n";
      sendto(CS_udp_socket, msg, sizeof(msg) - 1, 0, (struct sockaddr *)&client_addr, client_len); 
    }
    else {
      static char msg[] = "ERR\n";
      sendto(CS_udp_socket, msg, sizeof(msg) - 1, 0, (struct sockaddr *)&client_addr, client_len);
    }

    exit(0);
  }
}

