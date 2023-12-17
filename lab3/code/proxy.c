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

#include "network.h"
#include "./libparser/picohttpparser.h"
#include "proxy.h"

int execHttpProxy(const int port) {
  int socketListener = -1;
  printf("proxy is starting ...\n");

  socketListener = initSocketListener(socketListener, port);
  if (socketListener == -1) {
    printf("execHttpProxy: error in init listener socket '%s'\n",
           strerror(errno));
    close(socketListener);
    return -1;
  }

  printf("Http proxy is ready for connections\n");

  while (1) {

    struct sockaddr_in addrClient;
    socklen_t addrClientLength = sizeof(addrClient);
    addrClient.sin_family = AF_INET;

    int clientSocjetFD =
        accept(socketListener, (struct sockaddr *)&addrClient,
               &addrClientLength);
    if (clientSocjetFD == -1) {
      printf("execHttpProxy: accept(): ''%s''\n", strerror(errno));
      continue;
    }

    pthread_attr_t attrs;
    if (pthread_attr_init(&attrs) != 0) {
      printf("execHttpProxy: pthread_attr_init(): ''%s''\n",
             strerror(errno));
      close(socketListener);
      return -1;
    }

    if (pthread_attr_setdetachstate(&attrs,
                                    PTHREAD_CREATE_DETACHED) != 0) {
      printf("execHttpProxy: pthread_attr_setdetachstate(): ''%s''\n",
             strerror(errno));
      pthread_attr_destroy(&attrs);
      close(clientSocjetFD);
      return -1; // or continue; ?????
    }

    int *args = malloc(sizeof(*args));
    if (!args) {
      printf(
          "execHttpProxy: can't alloc mem for thread args: ''%s''\n",
          strerror(errno));
      pthread_attr_destroy(&attrs);
      close(clientSocjetFD);
      exit(EXIT_FAILURE);
    }

    pthread_t thread;
    *args = clientSocjetFD;
    if (pthread_create(&thread, &attrs, handleRequestConnection,
                       args) != 0) {
      printf("execHttpProxy: pthread_create(): '%s'\n",
             strerror(errno));
      free(args);
      pthread_attr_destroy(&attrs);
      close(clientSocjetFD);
      continue;
    }

    pthread_attr_destroy(&attrs); // ?
  }
}

void *handleRequestConnection(void *args) {
  int clientSocjetFD = *((int *)args);
  int hostSocketFD = -1;
  struct sockaddr_in hostAddr;
  char hostIP[16] = {0};
  char hostPort[8] = {0};
  int readBytesFromClient = 0;
  int writeBytesToHost = 0;
  char *buffer = malloc(BUFFER_SIZE * sizeof(char));

  printf("\n got new connection request on socket %d\n",
         clientSocjetFD);

  // read from client
  readBytesFromClient = read(clientSocjetFD, buffer, BUFFER_SIZE);
  if (readBytesFromClient == -1) {
    printf("error: read() '%s'\n", strerror(errno));

    close(clientSocjetFD);
    free(buffer);
    return NULL;
  } else if (readBytesFromClient == 0) {
    printf("connection on socket %d has been lost\n", clientSocjetFD);
    close(clientSocjetFD);
    free(buffer);
    return NULL;
  }

  int resOfParsing = parseHttpRequest(
      buffer, readBytesFromClient, hostIP, sizeof(hostIP), hostPort);
  if (resOfParsing == -1) {
    printf("error in parseHttpRequest()\n");
    close(clientSocjetFD);
    free(buffer);
    return NULL;
  }

  printf("trying connect to host: '%s':'%s' on socket %d\n", hostIP,
         hostPort, clientSocjetFD);

  hostSocketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (hostSocketFD == -1) {
    printf("error: socket(): '%s'\n", strerror(errno));
    close(clientSocjetFD);
    free(buffer);
    return NULL;
  }

  hostAddr.sin_family = AF_INET;

  if (inet_pton(AF_INET, hostIP, &(hostAddr.sin_addr)) == -1) {
    printf("error: inet_pton(): '%s'\n", strerror(errno));
    close(clientSocjetFD);
    close(hostSocketFD);
    free(buffer);
    return NULL;
  }

  hostAddr.sin_port = htons(atoi(hostPort));

  if (connect(hostSocketFD, (struct sockaddr *)&hostAddr,
              sizeof(hostAddr)) == -1) {
    printf("error: connect(): '%s'\n", strerror(errno));
    close(clientSocjetFD);
    close(hostSocketFD);
    free(buffer);
    return NULL;
  }

  printf("connection to host '%s':'%s' on socket %d has been "
         "successful\n",
         hostIP, hostPort, clientSocjetFD);

  exchangeData(buffer, hostSocketFD, clientSocjetFD, writeBytesToHost,
               readBytesFromClient);

  printf("closing connection on socket %d\n", clientSocjetFD);

  close(clientSocjetFD);
  close(hostSocketFD);
  free(buffer);
  return NULL;
}
