#include "common.h"

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

static char CS_name[128];
// static char SS_name[128]; 

static int CS_udp_socket = INVALID_SOCKET;
static int CS_tcp_socket = INVALID_SOCKET;
// static int SS_tcp_socket = INVALID_SOCKET;
static int CS_port = 58000 + NG;
// static int SS_port = 59000;

static struct hostent *CS_host;
// static struct hostent *SS_host;
static struct sockaddr_in CS_addr;
// static struct sockaddr_in SS_addr;
static struct sockaddr *CS_addr_ptr = (struct sockaddr *)&CS_addr;
// static struct sockaddr *SS_addr_ptr = (struct sockaddr *)&SS_addr;

// =============== Forward declarations ==================================== 

void init_connections(void);
void send_list_command(void);
void send_retrieve_command(const char *file);
void send_upload_command(const char *file);

// =============== Our main function ======================================= 

int main(int argc, char *argv[]) {
  // Skip executable path 
  --argc, ++argv;

  parse_args(argc, argv, &CS_port, CS_name);
  init_connections();

  for (;;) {
    printf("> ");
    fflush(stdout);

    char command[32] = {0}, arg[256] = {0}, line[1025] = {0};
    fgets(line, sizeof(line), stdin);
    if (feof(stdin)) {
      break;
    }
    if (ferror(stdin)) {
      HANDLE_ERRNO("Error reading stdin");
    }

    int ret = sscanf(line, "%s %s", command, arg);

    if (ret == 1 && strcmp(command, "list") == 0) {
      send_list_command();
    }
    else if (ret == 1 && strcmp(command, "exit") == 0) {
      break;
    }
    else if (strcmp(command, "retrieve") == 0) {
      send_retrieve_command(arg);
    }
    else if (strcmp(command, "upload") == 0) {
      send_upload_command(arg);
    }
    else {
      printf("Invalid command.\n");
    }
  }

  close(CS_udp_socket);
  close(CS_tcp_socket);
  // close(SS_tcp_socket);

  return 0;
}

// ========================================================================= 

void init_connections(void) {
  // We use UDP for the list command 
  CS_udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (CS_udp_socket == INVALID_SOCKET) {
    HANDLE_ERRNO("Failed to create UDP socket");
  }

  // ... and TCP for the upload command 
  CS_tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (CS_tcp_socket == INVALID_SOCKET) {
    HANDLE_ERRNO("Failed to create TCP socket");
  }

  // Uninitialized central server name, use own name 
  if (CS_name[0] == '\0' && gethostname(CS_name, sizeof(CS_name)) == -1) {
    HANDLE_ERRNO("Failed to get host name");
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
  if (connect(CS_tcp_socket, CS_addr_ptr, sizeof(CS_addr)) == -1) {
    HANDLE_ERRNO("Failed to connect to central server");
  }
}


// ========================================================================= 

void send_list_command() {
  static const char lst[] = "LST\n";
  if (sendto(CS_udp_socket, lst, sizeof(lst) - 1, 0, CS_addr_ptr, sizeof(CS_addr)) == -1) {
    HANDLE_ERRNO("Failed to send LST command");
  }

  // TODO: receive message, pretty-print it
}

// ========================================================================= 

void send_upload_command(const char *filename) { /* TODO */ (void)filename; }

// ========================================================================= 

void send_retrieve_command(const char *filename) { /* TODO */ (void)filename; }

