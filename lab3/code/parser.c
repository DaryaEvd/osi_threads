#include "parser.h"
#include "./libparser/picohttpparser.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE_PORT_HTTP "80"
#define HEADER_SIZE 256

int parseHttpRequest(char *buffer, ssize_t bufferLength, char *ip,
                     int lengthIP, char *port) {

  Request_t *request = malloc(sizeof(*request));
  if (!request) {
    printf("parseHttpRequest: '%s'\n", strerror(errno));
    return -1;
  }
  request->numHeaders = AMOUNT_HEADERS;
  request->lengthBuf = bufferLength;

  int errorParse = phr_parse_request(
      buffer, request->lengthBuf, &request->method,
      &request->lengthMethod, &request->path, &request->lengthPath,
      &request->minorVer, request->headers, &request->numHeaders, 0);

  if (errorParse == -1) {
    printf("parseHttpRequest: phr_parse_request() '%s'\n",
           strerror(errno));
    free(request);
    return -1;
  } else if (errorParse == -2) {
    printf("your request is not from browser. Go on!\n");
  } else {
    printf("your request is from browser. Go on!\n");
  }

  displayParsedRequestData(errorParse, request);

  // here we prevent all methods except GET
  if (!(request->method[0] == 'G' && request->method[1] == 'E' &&
        request->method[2] == 'T')) {
    printf("you can't exec commands except GET !!!! \n");
    free(request);
    return -1;
  }

  char currArrHeaders[HEADER_SIZE] = {0};

  if (request->numHeaders != 0) {
    displayHeader(request->headers, request->numHeaders, buffer,
                  currArrHeaders, request->lengthBuf);
  }

  parseHeaderPort(currArrHeaders, port);
  sscanf(request->path, "http://%99[^/]", ip);

  free(request);

  printf("------trying to resolve ip: '%s'\n", ip);
  printf("-------on port '%s'\n", port);

  struct addrinfo *result = NULL;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));

  int errResolver = resolveDomainName(hints, ip, lengthIP, result);
  if (errResolver == -1) {
    printf("parseHttpRequest: can't resolve a domain name\n");
    freeaddrinfo(result);
    result = NULL;
    return -1;
  }

  return 0;
}

void parseHeaderPort(char *currArrHeaders, char *port) {
  if (strstr(currArrHeaders, ":") != NULL) {
    strcpy(port, strtok(NULL, ":"));
  } else {
    strcpy(port, BASE_PORT_HTTP);
  }
}

int resolveDomainName(struct addrinfo hints, char *ip, int lengthIP,
                      struct addrinfo *result) {
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  int ret = -1;

  if (getaddrinfo(ip, "http", &hints, &result) != 0) {
    printf("parseHttpRequest: getaddrinfo(): '%s'\n",
           strerror(errno));
    return ret;
  }

  memset(ip, 0, lengthIP);

  for (struct addrinfo *addrInf = result; addrInf != NULL;
       addrInf = addrInf->ai_next) {
    struct sockaddr_in *currAddr =
        (struct sockaddr_in *)addrInf->ai_addr;
    void *addr = &(currAddr->sin_addr);

    if (inet_ntop(AF_INET, addr, ip, lengthIP) != NULL) {
      ret = 0;
      break;
    } else {
      printf("parseHttpRequest: inet_ntop(): '%s'\n",
             strerror(errno));
      ret = -1;
      break;
    }
  }

  freeaddrinfo(result);
  return ret;
}

void displayHeader(struct phr_header *headers, size_t numHeaders,
                   char *buffer, char *currArrHeaders,
                   ssize_t bufLength) {

  printf("----- start headers show ---------------------- \n");
  printf("%s", buffer);
  printf("--------------------- end headers show ------\n\n");
}

void displayParsedRequestData(int numBytes, Request_t *request) {
  printf("### res of parsing request starts ///\n");

  printf("request is '%d' bytes long\n", numBytes);
  printf("method is '%.*s'\n", (int)request->lengthMethod,
         request->method);
  printf("path is '%.*s'\n", (int)request->lengthPath, request->path);
  printf("HTTP version is '1.%d'\n", request->minorVer);
  printf("number of headers: '%ld'\n", request->numHeaders);

  printf("/// res of parsing request ends ###\n\n");
}
