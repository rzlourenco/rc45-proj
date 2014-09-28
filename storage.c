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

static int CS_fd = -1;

static int CS_port = 58000 + NG;
static int SS_port = 59000;

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
    CS_fd = setup_tcp_server(SS_port, &SS_addr);

    if (CS_fd == -1)
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

    int fd = accept(CS_fd, (struct sockaddr *)&client_addr, &client_len);

    if ((fd == -1)
    {
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