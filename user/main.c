#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

// Some globals 

#define INVALID_SOCKET (-1)
// Not needed, defined in the Makefile (-DNG=10)
// #define NG 10

static char CS_name[128];
// static char SS_name[128]; 

static int CS_udp_socket = INVALID_SOCKET;
static int CS_tcp_socket = INVALID_SOCKET;
static int CS_port = 58000 + NG;

static struct hostent *CS_host;
static struct sockaddr_in CS_addr;

// =============== Forward declarations ==================================== 

void init_connections(void);
void parse_args(int argc, char *argv[]);

// =============== Our main function ======================================= 

int main(int argc, char *argv[]) {
  // Skip executable path 
  --argc, ++argv;

  parse_args(argc, argv);
  init_connections();

  close(CS_udp_socket);
  close(CS_tcp_socket);

  return 0;
}

// ========================================================================= 

void init_connections(void) {
  // We use UDP for the list command 
  CS_udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (CS_udp_socket == INVALID_SOCKET) {
    fprintf(stderr, "Failed to create UDP socket: %s\n", strerror(errno));
    exit(1);
  }

  // ... and TCP for the upload command 
  CS_tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (CS_tcp_socket == INVALID_SOCKET) {
    fprintf(stderr, "Failed to create TCP socket: %s\n", strerror(errno));
    exit(1);
  }

  // Uninitialized central server name, use own name 
  if (CS_name[0] == '\0' && gethostname(CS_name, sizeof(CS_name)) == -1) {
    fprintf(stderr, "Failed to get own hostname: %s\n", strerror(errno));
    exit(1);
  }

  CS_host = gethostbyname(CS_name);  
  if (CS_host == NULL) {
    fprintf(stderr, "Failed to get host by name (%s): %s\n", CS_name, hstrerror(h_errno));
    exit(1);
  }

  CS_addr.sin_addr.s_addr = ((struct in_addr *)(CS_host->h_addr_list[0]))->s_addr;
  CS_addr.sin_family = AF_INET;
  CS_addr.sin_port = htons(CS_port);

  // Initialize TCP connection
  if (connect(CS_tcp_socket, (struct sockaddr *)&CS_addr, sizeof(CS_addr)) == -1) {
    fprintf(stderr, "Failed to connect to central server: %s\n", strerror(errno));
    exit(1);
  }
}

// ========================================================================= 

void parse_args(int argc, char *argv[]) {
  int i = 0;
  while (i < argc) {
    if (strcmp(argv[i], "-n") == 0 && i+1 < argc) {
      // FIXME: potencial buffer overflow 
      strcpy(CS_name, argv[i+1]);
      i += 2;
      continue;
    }
    else if (strcmp(argv[i], "-p") == 0 && i+1 < argc) {
      long l = strtol(argv[i+1], NULL, 10);
      CS_port = l;      
      i += 2;
      continue;
    }
    ++i;
  }
}

