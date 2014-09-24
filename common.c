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

