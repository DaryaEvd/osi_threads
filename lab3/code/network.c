#include "network.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

int initSocketListener(int socketListener, int port) {
  struct sockaddr_in addrProxy;
  addrProxy.sin_family = AF_INET;
  addrProxy.sin_port = htons(port);
  addrProxy.sin_addr.s_addr = inet_addr("127.0.0.1");

  socketListener = socket(AF_INET, SOCK_STREAM, 0);
  if (socketListener == -1) {
    printf("initSocketListener: socket(): '%s'\n", strerror(errno));
    return -1;
  }

  int enableReusing = 1;
  if (setsockopt(socketListener, SOL_SOCKET, SO_REUSEADDR,
                 &enableReusing, sizeof(enableReusing)) < 0) {
    printf("initSocketListener: setsockopt(): '%s'\n",
           strerror(errno));
    return -1;
  }

  if (bind(socketListener, (struct sockaddr *)&addrProxy,
           sizeof(addrProxy)) == -1) {
    printf("initSocketListener: bind(): '%s'\n", strerror(errno));
    return -1;
  }

  int maxAmountConnection = 100;
  if (listen(socketListener, maxAmountConnection) == -1) {
    printf("initSocketListener: listen(): '%s'\n", strerror(errno));
    return -1;
  }

  return socketListener;
}

int exchangeData(char *buffer, int hostSocketFD, int clientSocketFD,
                 int writeBytesToHost, int readBytesFromClient) {
  // write to host
  writeBytesToHost = write(hostSocketFD, buffer, readBytesFromClient);
  if (writeBytesToHost == -1) {
    printf("error: write(): '%s'\n", strerror(errno));
    return -1;
  }

  do {
    // read from host
    readBytesFromClient = read(hostSocketFD, buffer, BUFFER_SIZE);
    if (readBytesFromClient == -1) {
      printf("error: read(): '%s'\n", strerror(errno));
      return -1;
    } else if (readBytesFromClient == 0) {
      break;
    }

    // write response to client
    writeBytesToHost =
        write(clientSocketFD, buffer, readBytesFromClient);
    if (writeBytesToHost == -1) {
      printf("error: write(): '%s'\n", strerror(errno));
      return -1;
    }

  } while (readBytesFromClient > 0);

  return 0;
}
