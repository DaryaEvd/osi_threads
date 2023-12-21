#ifndef PARSER_H__
#define PARSER_H__

#include "./libparser/picohttpparser.h"
#include <netdb.h>
#include <unistd.h>

#define AMOUNT_HEADERS 100

typedef struct Request {
  struct phr_header headers[AMOUNT_HEADERS];
  size_t numHeaders;
  const char *method;
  const char *path;
  int minorVer;
  size_t lengthMethod;
  size_t lengthPath;
  size_t lengthBuf;
  size_t lengthPrevBuf;
} Request_t;

int parseHttpRequest(char *buffer, ssize_t bufferLength, char *ip,
                     int lengthIP, char *port);

void displayHeader(struct phr_header *headers, size_t numHeaders,
                   char *buffer, char *currArrHeaders, ssize_t bufLength);

void parseHeader(char *currArrHeaders, char *ip, char *port);

int resolveDomainName(struct addrinfo hints, char *ip, int lengthIP,
                      struct addrinfo *result);

#endif // PARSER_H__
