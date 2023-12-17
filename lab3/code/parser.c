#include "parser.h"
#include "./libparser/picohttpparser.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE_PORT_HTTP "80"
#define HEADER_SIZE 256

int parseHttpRequest(char *buffer, int bufferLength, char *ip,
                     int lengthIP, char *port) {

  Request_t *request = malloc(sizeof(*request));
  request->numHeaders = AMOUNT_HEADERS;

  char currArrHeaders[HEADER_SIZE];

  int errorParse = phr_parse_request(
      buffer, bufferLength, &request->method, &request->lengthMethod,
      &request->path, &request->lengthPath, &request->minorVer,
      request->headers, &request->numHeaders, 0);

  if (errorParse == -1) {
    printf("parseHttpRequest: phr_parse_request() ''%s''\n",
           strerror(errno));
    return -1;
  }

  displayHeader(request->headers, request->numHeaders, buffer,
                currArrHeaders);

  // if (strstr(currArrHeaders, ":") != NULL) {
  //   strcpy(ip, strtok(currArrHeaders, ":"));
  //   strcpy(port, strtok(NULL, ":"));
  // } else {
  //   strcpy(ip, currArrHeaders);
  //   strcpy(port, BASE_PORT_HTTP);
  // }
  parseHeader(currArrHeaders, ip, port);

  if (strcmp(port, BASE_PORT_HTTP) != 0) {
    printf("warning: get port which is default for unsupported "
           "protocol\n");
    // close()
    return -1; // ???
    // return -2;
  }

  printf("trying to resolve ip: ''%s''\n", ip);

  struct addrinfo *result = NULL;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));

  int errResolver = resolveDomainName(hints, ip, lengthIP, result);
  if (errResolver == -1) {
    printf("parseHttpRequest: can't resolve a domain name\n");
    return -1;
  }

  return 0;
}

void displayHeader(struct phr_header *headers, size_t numHeaders,
                   char *buffer, char *currArrHeaders) {
  size_t i;
  for (i = 0; i < numHeaders; i++) {
    if (strncmp(headers[i].name, "Host", headers[i].name_len) == 0) {
      break;
    }
  }

  printf("headers: '%s'\n", headers->name);

  sprintf(currArrHeaders, "%.*s", (int)headers[i].value_len,
          headers[i].value);

  printf("----- start headers show ---------------------- \n");
  printf("'%s'", buffer);
  printf("--------------------- end headers show ------\n\n");
}

void parseHeader(char *currArrHeaders, char *ip, char *port) {
  if (strstr(currArrHeaders, ":") != NULL) {
    strcpy(ip, strtok(currArrHeaders, ":"));
    strcpy(port, strtok(NULL, ":"));
  } else {
    strcpy(ip, currArrHeaders);
    strcpy(port, BASE_PORT_HTTP);
  }
}

int resolveDomainName(struct addrinfo hints, char *ip, int lengthIP,
                      struct addrinfo *result) {
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  
  if (getaddrinfo(ip, "http", &hints, &result) != 0) {
    printf("parseHttpRequest: getaddrinfo(): ''%s''\n",
           strerror(errno));
    return -1;
  }

  memset(ip, 0, lengthIP);

  for (struct addrinfo *addrInf = result; addrInf != NULL;
       addrInf = addrInf->ai_next) {
    struct sockaddr_in *currAddr =
        (struct sockaddr_in *)addrInf->ai_addr;
    void *addr = &(currAddr->sin_addr);

    if (inet_ntop(AF_INET, addr, ip, lengthIP) != NULL) {
      freeaddrinfo(result);
      return 0;
    } else {
      printf("parseHttpRequest: inet_ntop(): ''%s''\n",
             strerror(errno));
      freeaddrinfo(result);
    }
  }
  return -1;
}
