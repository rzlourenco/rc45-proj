#include "common.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

static char SS_name[128]; 

static int CS_tcp_socket = -1;
static int SS_tcp_socket = -1;
static int CS_port = 58000 + NG;
static int SS_port = 59000;

static struct sockaddr_in SS_addr;

static struct sockaddr *SS_addr_ptr = (struct sockaddr *)&SS_addr;

void tcp_loop(void);

int main(int argc, char *argv[])
{
  
  // Make sure child processes are reaped by the kernel
  signal(SIGCHLD, SIG_IGN);

  parse_args(argc, argv, &SS_port, NULL);

  pid_t tcp_server = fork();

  if (tcp_server == 0)
  {
    SS_tcp_socket = setup_tcp_server(SS_port, &SS_addr);

    if (SS_tcp_socket == -1)
    {
      E("Could not create TCP socket (%s)", strerror(errno));
    }
    tcp_loop();
  }

  else
  {
    E("Could not fork TCP server (%s)", strerror(errno));
  }

  return 0;
}

void tcp_loop()
{
  for (;;)
  {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr);

    int fd_client = accept(SS_tcp_socket, (struct sockaddr *)&client_addr, &client_len);
    int fd_server = accept(SS_tcp_socket, (struct sockaddr *)&server_addr, &server_len);

    if ((fd_client == -1) || (fd_server) == -1) {
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