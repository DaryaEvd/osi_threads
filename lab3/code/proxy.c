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

    int clientSocketFD =
        accept(socketListener, (struct sockaddr *)&addrClient,
               &addrClientLength);
    if (clientSocketFD == -1) {
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
      close(clientSocketFD);
      return -1;
    }

    int *args = malloc(sizeof(*args));
    if (!args) {
      printf(
          "execHttpProxy: can't alloc mem for thread args: ''%s''\n",
          strerror(errno));
      pthread_attr_destroy(&attrs);
      close(clientSocketFD);
      return -1;
      // exit(EXIT_FAILURE);
    }

    pthread_t thread;
    *args = clientSocketFD;
    if (pthread_create(&thread, &attrs, handleRequestConnection,
                       args) != 0) {
      printf("execHttpProxy: pthread_create(): '%s'\n",
             strerror(errno));
      pthread_attr_destroy(&attrs);
      close(clientSocketFD);
      free(args);

      continue;
      // return -1;
    }

    pthread_attr_destroy(&attrs);
  }
}

int connect_to_target(char *host, int port) {
  struct hostent *he;
  struct sockaddr_in server_addr;
  int sock;

  // Получение информации о хосте
  if ((he = gethostbyname(host)) == NULL) {
    herror("gethostbyname failed");
    return -1;
  }

  // Создание сокета
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket creation error");
    return -1;
  }

  // Настройка параметров сервера
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr = *((struct in_addr *)he->h_addr_list[0]);
  memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

  // Подключение к серверу
  if (connect(sock, (struct sockaddr *)&server_addr,
              sizeof(server_addr)) < 0) {
    perror("Connection Failed");
    return -1;
  }

  return sock;
}

void *handleRequestConnection(void *args) {
  // /*
  int clientSocketFD = *((int *)args);
  int hostSocketFD = -1;
  struct sockaddr_in hostAddr;
  char hostIP[16] = {0};
  char hostPort[8] = {0};
  ssize_t readBytesFromClient = 0;
  ssize_t writeBytesToHost = 0;

  char *buffer = malloc(BUFFER_SIZE * sizeof(char));
  if (!buffer) {
    printf("handleRequestConnection: malloc() '%s'\n",
           strerror(errno));
    return NULL;
  }

  printf("\n got new connection request on socket %d\n",
         clientSocketFD);

  // read from client
  readBytesFromClient = read(clientSocketFD, buffer, BUFFER_SIZE);
  if (readBytesFromClient == -1) {
    printf("handleRequestConnection: read() '%s'\n", strerror(errno));
    free(buffer);
    close(clientSocketFD);
    return NULL;
  } else if (readBytesFromClient == 0) {
    printf("connection on socket %d has been lost\n", clientSocketFD);
    free(buffer);
    close(clientSocketFD);
    return NULL;
  }

  int resOfParsing = parseHttpRequest(
      buffer, readBytesFromClient, hostIP, sizeof(hostIP), hostPort);
  if (resOfParsing == -1) {
    printf("error in parseHttpRequest()\n");
    free(buffer);
    close(clientSocketFD);
    return NULL;
  }

  printf("trying connect to host: '%s:%s' on socket %d\n", hostIP,
         hostPort, clientSocketFD);

  hostSocketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (hostSocketFD == -1) {
    printf("handleRequestConnection: socket(): '%s'\n",
           strerror(errno));
    free(buffer);
    close(clientSocketFD);
    return NULL;
  }

  hostAddr.sin_family = AF_INET;

  if (inet_pton(AF_INET, hostIP, &(hostAddr.sin_addr)) == -1) {
    printf("handleRequestConnection: inet_pton(): '%s'\n",
           strerror(errno));
    free(buffer);
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
    close(clientSocketFD);
    close(hostSocketFD);
    return NULL;
  }

  printf("connection to host '%s':'%s' on socket %d has been "
         "successful\n",
         hostIP, hostPort, clientSocketFD);

  exchangeData(buffer, hostSocketFD, clientSocketFD, writeBytesToHost,
               readBytesFromClient);

  printf("closing connection on socket %d\n", clientSocketFD);

  free(buffer);
  close(clientSocketFD);
  close(hostSocketFD);
  return NULL;

  // */
}

/*
  int client_sock = *(int *)args;
  char buffer[BUFFER_SIZE];
  int target_sock, read_bytes;
  int data_size = 0;
  char *data = NULL;

  // Чтение запроса от клиента
  while (1) {
    read_bytes = recv(client_sock, buffer, BUFFER_SIZE, 0);
    if (read_bytes <= 0) {
      break;
    }

    data = realloc(data, data_size + read_bytes);
    memcpy(data + data_size, buffer, read_bytes);
    data_size += read_bytes;

    // Проверка на наличие CRLF
    if (strstr(data, "\r\n\r\n") != NULL) {
      break;
    }
  }

  // Извлечение имени хоста из запроса
  char host[_SC_HOST_NAME_MAX + 1] = {0};
  char *host_line_start = strstr(data, "Host: ");
  if (!host_line_start) {
    host_line_start = strstr(
        data, "host: "); // Попытка найти заголовок в другом
  регистре
  }

  if (host_line_start) {
    char *host_line_end = strstr(host_line_start, "\r\n");
    if (!host_line_end) {
      host_line_end = strstr(host_line_start, "\n");
    }

    if (host_line_end) {
      int host_length = host_line_end - (host_line_start + 6);
      strncpy(host, host_line_start + 6, host_length);
      host[host_length] =
          '\0'; // Обеспечиваем правильное завершение строки
    } else {
      printf("Host header line end not found.\n");
    }
  } else {
    printf("Host header not found in request.\n");
    close(client_sock);
    return NULL;
  }

  printf("Connecting to host: %s\n", host);

  // Подключение к целевому серверу
  target_sock = connect_to_target(
      host, 80); // Используем PORT, определенный в начале программы
  if (target_sock < 0) {
    close(client_sock);
    return NULL;
  }


  // Передача запроса на целевой сервер
  send(target_sock, data, data_size, 0);
  free(data);

  // Передача ответа обратно клиенту
  while ((read_bytes = recv(target_sock, buffer, BUFFER_SIZE, 0)) >
         0) {
    send(client_sock, buffer, read_bytes, 0);
  }

  printf("------connection to  %s %d\n", host, target_sock);

  // Закрытие соединений
  close(target_sock);
  close(client_sock);
  return NULL;
  */