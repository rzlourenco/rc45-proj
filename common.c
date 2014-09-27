#include "common.h"
#include <assert.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void parse_args(int argc, char *argv[], int *port, char *name) {
  int i = 0;
  while (i < argc) {
    if (strcmp(argv[i], "-n") == 0 && i+1 < argc) {
      if (name != NULL) { strcpy(name, argv[i+1]); }
      i += 2;
      continue;
    }
    else if (strcmp(argv[i], "-p") == 0 && i+1 < argc) {
      long l = strtol(argv[i+1], NULL, 10);
      if (port != NULL) { *port = l; }
      i += 2;
      continue;
    }
    ++i;
  }
}

// ========================================================================= 

int setup_socket(int family, int type, int protocol, in_port_t port, struct sockaddr_in *outaddr) {
  int fd = socket(family, type, protocol);
  if (fd == -1) {
    return -1;
  }

  struct sockaddr_in addr;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  assert(outaddr);
  *outaddr = addr;

  return fd;
}

int setup_tcp_server(in_port_t port, struct sockaddr_in *outaddr) {
  struct sockaddr_in addr;
  if (outaddr == NULL) {
    outaddr = &addr;
  }

  int fd = setup_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP, port, outaddr);
  if (fd == -1) {
    return -1;
  }

  if (bind(fd, (struct sockaddr *)outaddr, sizeof(*outaddr)) == -1) {
    close(fd);
    return -1;
  }

  if (listen(fd, 8) == -1) {
    close(fd);
    return -1;
  }

  return fd;
}

int setup_udp_server(in_port_t port, struct sockaddr_in *outaddr) {
  struct sockaddr_in addr;
  if (outaddr == NULL) {
    outaddr = &addr;
  }

  int fd = setup_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, port, outaddr);

  if (bind(fd, (struct sockaddr *)outaddr, sizeof(*outaddr)) == -1) {
    close(fd);
    return -1;
  }

  return fd;
}

// ========================================================================= 

int connect_common(int family, int type, int protocol, const char *hostname, in_port_t port, struct sockaddr_in *outaddr) {
  struct sockaddr_in addr;
  if (outaddr == NULL) {
    outaddr = &addr;
  }

  int fd = setup_socket(family, type, protocol, port, outaddr);

  char realhost[128] = {0};
  if (hostname == NULL) {
    if (gethostname(realhost, sizeof(realhost)) == -1) {
      close(fd);
      return -1;
    }

    hostname = realhost;
  }

  struct hostent *host = gethostbyname(hostname);
  if (host == NULL) {
    close(fd);
    return -1;
  }

  outaddr->sin_addr.s_addr = ((struct in_addr *)(host->h_addr_list[0]))->s_addr;
  outaddr->sin_family = family;
  outaddr->sin_port = htons(port);

  return fd;
}

int connect_tcp(const char *hostname, in_port_t port, struct sockaddr_in *outaddr) {
  struct sockaddr_in addr;
  if (outaddr == NULL) {
    outaddr = &addr;
  }

  int fd = connect_common(AF_INET, SOCK_STREAM, IPPROTO_TCP, hostname, port, outaddr);

  if (connect(fd, (struct sockaddr *)outaddr, sizeof(*outaddr)) == -1) {
    close(fd);
    return -1;
  }

  return fd;
}

int connect_udp(const char *hostname, in_port_t port, struct sockaddr_in *outaddr) {
  struct sockaddr_in addr;
  if (outaddr == NULL) {
    outaddr = &addr;
  }

  return connect_common(AF_INET, SOCK_DGRAM, IPPROTO_UDP, hostname, port, outaddr);
}

// ========================================================================= 

#if 0
// This function reads until EOF into memory. May not be appropriate for
// large messages.
ssize_t full_read(int fd, char **out) {
  char buf[8 * 1024], *data = NULL;
  ssize_t ret = 0, allocd = 0;

  do {
    printf("DEBUG: calling read...\n");
    ssize_t readBytes = read(fd, buf, sizeof(buf));
    printf("DEBUG: read %zd bytes\n", readBytes);

    // An error ocurred
    if (readBytes == -1) {
      free(data);
      return -1;
    }
    if (readBytes == 0 && data != NULL) {
      break; 
    }
    printf("DEBUG: not eof, no error ocurred, proceeding\n");

    if (data == NULL) {
      printf("DEBUG: data is NULL, allocating\n");
      data = calloc(readBytes, sizeof(char));    
      allocd = readBytes;
      printf("DEBUG: allocated %zu bytes\n", allocd);
    }
    if (ret + readBytes > allocd) {
      printf("DEBUG: SHOULD NOT HAPPEN\n");
#define MAX(a, b) ((a) > (b) ? (a) : (b))
      allocd = MAX(ret + readBytes, 4 * allocd / 3);
      data = realloc(data, allocd);
#undef MAX
    }

    strncpy(data + ret, buf, readBytes);
    ret += readBytes;
  } while (1);

  printf("DEBUG: exited loop\n");

  *out = data;
  return ret;
}

#endif

