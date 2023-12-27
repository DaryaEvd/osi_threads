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
  }

  // here we prevent all methods except GET
  char *startOfSecondWordInBuffer = strchr(buffer, ' ');
  size_t lengthOfFirst = startOfSecondWordInBuffer - buffer;
  char *firstInBuffer =
      (char *)malloc((lengthOfFirst + 1) * sizeof(char));
  if (!firstInBuffer) {
    printf("parseHttpRequest: malloc() '%s'", strerror(errno));
    free(request);
    return -1;
  }
  strncpy(firstInBuffer, buffer, lengthOfFirst);

  if (!(firstInBuffer[0] == 'G' && firstInBuffer[1] == 'E' &&
        firstInBuffer[2] == 'T')) {
    printf("you can't exec commands except GET !!!! \n");
    free(request);
    free(firstInBuffer);
    return -1;
  }

  free(firstInBuffer);

  char currArrHeaders[HEADER_SIZE];
  displayHeader(request->headers, request->numHeaders, buffer,
                currArrHeaders, request->lengthBuf);

  free(request);

  parseHeader(currArrHeaders, ip, port);

  printf("------trying to resolve ip: '%s'\n", ip);

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

void displayHeader(struct phr_header *headers, size_t numHeaders,
                   char *buffer, char *currArrHeaders,
                   ssize_t bufLength) {
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
  printf("%s", buffer);
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
