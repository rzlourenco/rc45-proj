#include "common.h"

#include <arpa/inet.h>
#include <assert.h>
#include <dirent.h>
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
const char *workdir = NULL;

void tcp_loop(void);
void udp_loop(void);

int main(int argc, char *argv[]) {
  // Make sure child processes are reaped by the kernel
  signal(SIGCHLD, SIG_IGN);

  // We are depending on a Linux extension here. This is not POSIX compliant
  workdir = getcwd(NULL, 0);

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

void handle_upload(int fd, char *msg);
void tcp_loop() {
  for (;;) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int fd = accept(CS_tcp_socket, (struct sockaddr *)&client_addr, &client_len);

    if (fd == -1) {
      E("could not accept TCP connection (%s)", strerror(errno));
    }

    pid_t pid = fork();
    if (pid == -1) {
      E("could not fork TCP server (%s)", strerror(errno));
    }

    // Leave client handling to child process
    if (pid != 0) {
      close(fd);
      continue;
    }

    // Handle client requests
    char buf[64*1024] = {0};
    if (read(fd, buf, sizeof(buf)-1) == -1) {
      E("error reading from client (%s)", strerror(errno));
    }

    if (strncmp(buf, "UPR ", 4) == 0) {
      handle_upload(fd, buf);
    }
    else {
      static char resp[] = "AWR ERR\n";
      if (write(fd, resp, sizeof(resp)-1) == -1) {
        E("could not send response to client %x (%s)", client_addr.sin_addr.s_addr, strerror(errno));
      }
    }

    // Terminate process
    exit(0);
  }
}

void handle_upload(int fd, char *msg) {(void)fd; (void)msg;}

void handle_LST(struct sockaddr_in *addr, socklen_t *addrlen);
void udp_loop() {
  for (;;) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    char msg[64 * 1024] = {0};
    if (recvfrom(CS_udp_socket, msg, sizeof(msg)-1, 0, (struct sockaddr *)&client_addr, &client_len) == -1) {
      E("Failed to receive data from client (%s)", strerror(errno));
    }

    pid_t child = fork();
    if (child == -1) {
      E("could not fork process (%s)", strerror(errno));
    }

    if (child != 0) {
      continue;
    }

    if (strncmp("LST\n", msg, 4) == 0) {
      handle_LST(&client_addr, &client_len);      
    }
    else {
      static char resp[] = "ERR\n";
      if (sendto(CS_udp_socket, resp, sizeof(resp) - 1, 0, (struct sockaddr *)&client_addr, client_len) == -1) {
        E("could not send response to client %x (%s)", client_addr.sin_addr.s_addr, strerror(errno));
      }
    }

    exit(0);
  }
}

void handle_LST(struct sockaddr_in *addr, socklen_t *addrlen) {
  DIR *dir = opendir(workdir);
  if (dir == NULL) {
    E("could not open directory %s (%s)", workdir, strerror(errno));
  }

  struct dirent *ent;
  char buf[64 * 1024] = {0};

  int totalBytes = snprintf(buf, sizeof(buf)-1, "AWL %s %d", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
  unsigned numFiles = 0;
  char *fileBuf[30] = {0};
  while ((ent = readdir(dir)) != NULL) {
    if ((ent->d_type & DT_REG) == DT_REG && numFiles < (sizeof(fileBuf)/sizeof(*fileBuf))) {
      // Skip "hidden" files
      if (ent->d_name[0] == '.') {
        continue;
      }

      fileBuf[numFiles++] = strdup(ent->d_name);
    }    
  }

  closedir(dir);

  totalBytes += snprintf(buf + totalBytes, sizeof(buf) - totalBytes - 1, " %u", numFiles);
  for (unsigned i = 0; i < numFiles; ++i) {
    totalBytes += snprintf(buf + totalBytes, sizeof(buf) - totalBytes - 1, " %s", fileBuf[i]);
    free(fileBuf[i]);
  }

  buf[totalBytes++] = '\n';

  // TODO: properly handle return value
  assert(sendto(CS_udp_socket, buf, totalBytes, 0, (struct sockaddr *)addr, *addrlen) != -1);
}

