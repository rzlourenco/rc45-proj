#include "common.h"

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static char CS_name[128] = "localhost";
static int CS_port = 58000 + NG;

void tcp_loop() {
  for (;;) {
    // newfd = accept
    // fork
    // if parent
    //   close newfd
    //   continue
    // handle newfd
  }
}

void udp_loop() {
  for (;;) {
  }
}

int main(int argc, char *argv[]) {
  parse_args(argc, argv, &CS_port, CS_name);
  // set up udp
  // set up tcp

  pid_t udp_server = fork();
  if (udp_server == 0) {
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

