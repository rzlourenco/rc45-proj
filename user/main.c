#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

// Some globals 

#define INVALID_SOCKET (-1)

static char CSname[128] = { 0 };
// static char SSname[128] = { 0 }; 

static int CS_udp_socket = INVALID_SOCKET;
static int CS_tcp_socket = INVALID_SOCKET;
static int CSport = 0;

// =============== Forward declarations ==================================== 

void init_connections(void);
void parse_args(int argc, char *argv[]);

// =============== Our main function ======================================= 

int main(int argc, char *argv[]) {
  // Skip executable path 
  --argc, ++argv;

  parse_args(argc, argv);
  init_connections();

  for (;;) {

  }

  close(CS_udp_socket);
  close(CS_tcp_socket);

  return 0;
}

// ========================================================================= 

void init_connections(void) {
  // We use UDP for the list command 
  CS_udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (CS_udp_socket == INVALID_SOCKET) {
    fprintf(stderr, "Failed to create UDP socket to central server: %s\n", strerror(errno));
    exit(1);
  }

  // ... and TCP for the upload command 
  CS_tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (CS_tcp_socket == INVALID_SOCKET) {
    fprintf(stderr, "Failed to create TCP socket to central server: %s\n", strerror(errno));
    exit(1);
  }

  // Uninitialized central server name, use own name 
  if (CSname[0] == '\0' && gethostname(CSname, sizeof(CSname)) == -1) {
    fprintf(stderr, "Failed to gethostname: %s\n", strerror(errno));
    exit(1);
  }

  
}

// ========================================================================= 

void parse_args(int argc, char *argv[]) {
  int i = 0;
  while (i < argc) {
    if (strcmp(argv[i], "-n") == 0 && i+1 < argc) {
      // FIXME: potencial buffer overflow 
      strcpy(CSname, argv[i+1]);
      i += 2;
      continue;
    }
    else if (strcmp(argv[i], "-p") == 0 && i+1 < argc) {
      long l = strtol(argv[i+1], NULL, 10);
      CSport = l;      
      i += 2;
      continue;
    }
    ++i;
  }
}

