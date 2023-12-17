#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "proxy.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: '%s' <port>\n", argv[0]);
    return 0;
  }

  for (int i = 0; argv[1][i] != '\0'; i++) {
    if (!isdigit(argv[1][i])) {
      printf("'%s' is not a valid positive number\n", argv[1]);
      return -1;
    }
  }

  int portServer = atoi(argv[1]);
  execHttpProxy(portServer);

  return 0;
}
