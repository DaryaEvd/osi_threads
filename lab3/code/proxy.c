#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "./libparser/picohttpparser.h"
#include "network.h"
#include "proxy.h"

int execHttpProxy(const int port) {
  printf("Http proxy is starting ...\n");

  int socketListener = -1;
  socketListener = initSocketListener(socketListener, port);
  if (socketListener == -1) {
    printf("execHttpProxy: error in init listener socket '%s'\n",
           strerror(errno));
    close(socketListener);
    return -1;
  }

  printf("Http proxy is ready for connections!\n");

  while (1) {
    struct sockaddr_in addrClient;
    socklen_t addrClientLength = sizeof(addrClient);
    addrClient.sin_family = AF_INET;

    int clientSocketFD =
        accept(socketListener, (struct sockaddr *)&addrClient,
               &addrClientLength);
    if (clientSocketFD == -1) {
      printf("execHttpProxy: accept(): '%s'\n", strerror(errno));
      continue;
    }

    pthread_attr_t attrs;
    if (pthread_attr_init(&attrs) != 0) {
      printf("execHttpProxy: pthread_attr_init(): '%s'\n",
             strerror(errno));
      close(socketListener);
      return -1;
    }

    if (pthread_attr_setdetachstate(&attrs,
                                    PTHREAD_CREATE_DETACHED) != 0) {
      printf("execHttpProxy: pthread_attr_setdetachstate(): '%s'\n",
             strerror(errno));
      pthread_attr_destroy(&attrs);
      close(clientSocketFD);
      return -1;
    }

    int *args = malloc(sizeof(int));
    if (!args) {
      printf("execHttpProxy: can't alloc mem for thread args: '%s'\n",
             strerror(errno));
      pthread_attr_destroy(&attrs);
      close(clientSocketFD);
      return -1;
    }

    pthread_t thread;
    *args = clientSocketFD;
    if (pthread_create(&thread, &attrs, handleRequestConnection,
                       args) != 0) {
      printf("execHttpProxy: pthread_create(): '%s'\n",
             strerror(errno));
      free(args);
      pthread_attr_destroy(&attrs);

      continue;
    }

    pthread_attr_destroy(&attrs);
  }
}

void *handleRequestConnection(void *args) {
  int clientSocketFD = *((int *)args);
  free(args);

  int hostSocketFD = -1;
  struct sockaddr_in hostAddr;
  char hostIP[16] = {0};
  char hostPort[8] = {0};

  char *buffer = calloc(BUFFER_SIZE, sizeof(char *));
  if (!buffer) {
    printf("handleRequestConnection: malloc() '%s'\n",
           strerror(errno));
    buffer = NULL;
    close(clientSocketFD);
    return NULL;
  }

  printf("\n----- got new connection request on socket %d\n",
         clientSocketFD);

  // read from client
  ssize_t readBytesFromClient =
      read(clientSocketFD, buffer, BUFFER_SIZE);
  if (readBytesFromClient == -1) {
    printf("handleRequestConnection: read() '%s'\n", strerror(errno));
    free(buffer);
    close(clientSocketFD);
    return NULL;
  } else if (readBytesFromClient == 0) {
    free(buffer);
    printf("connection on socket %d has been lost\n", clientSocketFD);
    close(clientSocketFD);
    return NULL;
  }

  printf("--- BUFFER STARTS --- \n");
  printf("%s", buffer);
  printf("--- BUFFER ENDS ------\n\n");

  int resOfParsing = parseHttpRequest(
      buffer, readBytesFromClient, hostIP, sizeof(hostIP), hostPort);
  if (resOfParsing == -1) {
    free(buffer);
    printf("----- close connection by crash on socket %d\n",
           clientSocketFD);
    close(clientSocketFD);
    return NULL;
  }

  printf("-------trying connect to host: '%s:%s' on socket '%d'\n",
         hostIP, hostPort, clientSocketFD);

  hostSocketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (hostSocketFD == -1) {
    printf("handleRequestConnection: socket(): '%s'\n",
           strerror(errno));
    free(buffer);
    printf("----- close connection on socket %d\n", clientSocketFD);
    close(clientSocketFD);
    return NULL;
  }

  hostAddr.sin_family = AF_INET;

  if (inet_pton(AF_INET, hostIP, &(hostAddr.sin_addr)) == -1) {
    printf("handleRequestConnection: inet_pton(): '%s'\n",
           strerror(errno));
    free(buffer);
    printf("----- close connection on socket %d\n", clientSocketFD);
    close(clientSocketFD);
    close(hostSocketFD);
    return NULL;
  }

  hostAddr.sin_port = htons(atoi(hostPort));

  if (connect(hostSocketFD, (struct sockaddr *)&hostAddr,
              sizeof(hostAddr)) == -1) {
    printf("handleRequestConnection: connect(): '%s'\n",
           strerror(errno));
    free(buffer);
    printf("----- close connection on socket %d\n", clientSocketFD);
    close(clientSocketFD);
    close(hostSocketFD);
    return NULL;
  }

  printf("connection to host '%s:%s' on socket '%d' has been "
         "successful\n",
         hostIP, hostPort, clientSocketFD);

  ssize_t writeBytesToHost = 0;
  if (exchangeData(buffer, hostSocketFD, clientSocketFD,
                   writeBytesToHost, readBytesFromClient) == -1) {
    free(buffer);
    close(clientSocketFD);
    close(hostSocketFD);
    return NULL;
  }

  printf("--- closing connection on socket %d\n", clientSocketFD);

  free(buffer);
  close(clientSocketFD);
  close(hostSocketFD);
  return NULL;
}
