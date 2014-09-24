#include "common.h"
#include <stdlib.h>
#include <string.h>

void parse_args(int argc, char *argv[], int *port, char *name) {
  int i = 0;
  while (i < argc) {
    if (strcmp(argv[i], "-n") == 0 && i+1 < argc) {
      // FIXME: potencial buffer overflow 
      strcpy(name, argv[i+1]);
      i += 2;
      continue;
    }
    else if (strcmp(argv[i], "-p") == 0 && i+1 < argc) {
      long l = strtol(argv[i+1], NULL, 10);
      *port = l;      
      i += 2;
      continue;
    }
    ++i;
  }
}

// ========================================================================= 

// This function reads until EOF into memory. May not be appropriate for
// large messages
char *receive_udp(int fd, struct sockaddr *addr) {
  // Allocate a temporary buffer sized 1024
  char *ret = malloc(1024 * sizeof(char)), buf[1024];
  size_t maxSize = 1024, curSize = 0;

  do {
    socklen_t addrLen = sizeof(struct sockaddr);
    int readBytes = recvfrom(fd, buf, sizeof(buf), 0, addr, &addrLen);

    // EOF
    if (readBytes == 0) {
      break;
    }
    else if (readBytes == -1) {
      HANDLE_ERRNO("Error receiving data via UDP");
    }

    if (curSize + readBytes > maxSize) {
      ret = realloc(ret, 2 * maxSize);
    }

    strncpy(ret + curSize, buf, sizeof(buf));
    curSize += readBytes;
  } while (1);

  // Make sure it ends in \0
  // FIXME: Potencially unsafe memory access
  ret[curSize++] = '\0';

  return ret;
}

