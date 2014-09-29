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

static int CS_udp_socket = -1;
static int CS_tcp_socket = -1;
static int SS_tcp_socket = -1;
static int CS_port = 58000 + NG;
static int SS_port = 0;

static struct sockaddr_in CS_addr;
static struct sockaddr_in SS_addr;
static struct sockaddr *CS_addr_ptr = (struct sockaddr *)&CS_addr;
static struct sockaddr *SS_addr_ptr = (struct sockaddr *)&SS_addr;

// =============== Forward declarations ==================================== 

void send_list_command(void);
void send_retrieve_command(const char *file);
void send_upload_command(const char *file);

// =============== Our main function ======================================= 

int main(int argc, char *argv[]) {
  // Skip executable path 
  --argc, ++argv;

  char CS_name[128] = {0};
  parse_args(argc, argv, &CS_port, CS_name);

  char *real_CS_name = NULL;
  if (CS_name[0] != '\0') {
    real_CS_name = CS_name;
  }

  CS_udp_socket = connect_udp(real_CS_name, CS_port, &CS_addr);
  if (CS_udp_socket == -1) {
    E("Could not connect to central server via UDP (%s)", strerror(errno));
  }

  CS_tcp_socket = connect_tcp(real_CS_name, CS_port, &CS_addr);
  if (CS_tcp_socket == -1) {
    E("Could not connect to central server via TCP (%s)", strerror(errno));
  }

  for (;;) {
    printf("> ");
    fflush(stdout);

    char command[32] = {0}, arg[256] = {0}, line[1025] = {0};
    fgets(line, sizeof(line), stdin);
    if (feof(stdin)) {
      break;
    }

    int ret = sscanf(line, "%s %s \n", command, arg);

    if (ret == 1 && strcmp(command, "list") == 0) {
      send_list_command();
    }
    else if (ret == 1 && strcmp(command, "exit") == 0) {
      break;
    }
    else if (ret == 2 && strcmp(command, "retrieve") == 0) {
      send_retrieve_command(arg);
    }
    else if (ret == 2 && strcmp(command, "upload") == 0) {
      send_upload_command(arg);
    }
    else {
      printf("Invalid command.\n");
    }
  }

  close(CS_udp_socket);
  close(CS_tcp_socket);
  close(SS_tcp_socket);

  return 0;
}

// ========================================================================= 

void handle_list_response(char *msg);
void send_list_command() {
  static const char lst[] = "LST\n";
  if (sendto(CS_udp_socket, lst, sizeof(lst) - 1, 0, CS_addr_ptr, sizeof(CS_addr)) == -1) {
    E("could request file list from central server (%s)", strerror(errno));
    return;
  }

  char answer[64 * 1024] = {0};
  socklen_t len = sizeof(CS_addr);
  ssize_t ret = recvfrom(CS_udp_socket, &answer, sizeof(answer), 0, CS_addr_ptr, &len);
  if (ret == -1) {
    E("could not receive file list from central server (%s)", strerror(errno));
  }
  else if (ret == 0) {
    E("the central server closed the connection");
  }

  handle_list_response(answer);

  if (SS_tcp_socket != -1) {
    close(SS_tcp_socket);
    SS_tcp_socket = -1;
    errno = 0;
  }

  SS_tcp_socket = connect_tcp(SS_name, SS_port, &SS_addr);
  if (SS_tcp_socket == -1) {
    E("Failed to connect to storage server (%s)", strerror(errno));
  }
}

void handle_list_response(char *msg) {
  char *s = strtok(msg, " "); assert(s);

  if (strcmp("AWL", s) != 0) {
    E("unexpected response to LST command from central server - %s", s);
  }

  s = strtok(NULL, " "); assert(s);
  // Copy hostname/IP
  strncpy(SS_name, s, sizeof(SS_name));
  SS_name[sizeof(SS_name) - 1] = '\0';
  D("SS hostname is %s", SS_name);

  // Read port
  s = strtok(NULL, " "); assert(s);
  SS_port = strtol(s, NULL, 10);
  D("SS port is %d", SS_port);

  // Read number of files
  s = strtok(NULL, " "); assert(s);
  int numFiles = strtol(s, NULL, 10);

  if (numFiles <= 0) {
    printf("No files present in server.\n");
    return;
  }

  // Pretty-print file list
  s = strtok(NULL, " ");
  printf("Files avaliable:\n");

  int i;
  for (i = 1; s != NULL && i <= numFiles; ++i) {
    printf("  %3d. %s\n", i, s);
    s = strtok(NULL, " ");
  }

  if (i < numFiles) {
    W("File list exceeded buffer length.");
  }
}

// ========================================================================= 

void send_upload_command(const char *filename) {
  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
    W("could not open file %s (%s)", filename, strerror(errno));
    return;
  }

  struct stat file_info;
  fstat(fd, &file_info);
  
  if (file_info.st_size > MAX_FILE_SIZE) {
    W("file size is greater than %d, upload aborted", MAX_FILE_SIZE);
    close(fd);
    return;
  }

  char buf[32 * 1024] = {0};
  int numBytes = snprintf(buf, sizeof(buf)-1, "UPR %s\n", filename);

  if (write(CS_tcp_socket, buf, numBytes) == -1) {
    E("could not check if file %s exists in server (%s)", filename, strerror(errno));
  }

  numBytes = read(CS_tcp_socket, buf, sizeof(buf)-1);
  if (numBytes == -1) {
    E("could not read response from server (%s)", strerror(errno));
  }
  buf[numBytes] = '\0';

  D("%s", buf);

  if (strncmp("AWR ", buf, 4) != 0) {
    E("unexpected response from server: %s", buf);
  }

  if (strncmp("dup\n", buf + 4, 4) == 0) {
    W("file already exists on server, not uploading");
    close(fd);
    return;
  }
  else if (strncmp("new\n", buf + 4, 4) != 0) {
    E("unexpected response from central server: %s", buf);
  }

  // Prepare upload command
  numBytes = snprintf(buf, sizeof(buf)-1, "UPC %zd ", file_info.st_size);
  numBytes += read(fd, buf + numBytes, MAX_FILE_SIZE);
  buf[numBytes++] = '\n';
  close(fd);

  if (write(CS_tcp_socket, buf, numBytes) == -1) {
    E("failed to upload %s (%s)", filename, strerror(errno));
  }
  
  numBytes = read(CS_tcp_socket, buf, sizeof(buf)-1);
  if (numBytes == -1) {
    E("could not read response from server (%s)", strerror(errno));
  }
  buf[numBytes] = '\0';

  D("%s", buf);

  if (strncmp("AWC ", buf, 4) != 0 || strncmp("ok\n", buf + 4, 3) != 0) {
    E("unexpected responser from server: %s", buf);
  }

  printf("File uploaded with success\n");
}

// ========================================================================= 

void send_retrieve_command(const char *filename) {
  // TODO
  (void)filename;
}

