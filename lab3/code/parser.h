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
} Request_t;

int parseHttpRequest(char *buffer, int bufferLength, char *ip,
                     int lengthIP, char *port);

void displayHeader(struct phr_header *headers, size_t numHeaders,
                   char *buffer, char *currArrHeaders);

void parseHeader(char *currArrHeaders, char *ip, char *port);

int resolveDomainName(struct addrinfo hints, char *ip, int lengthIP,
                      struct addrinfo *result);

#endif // PARSER_H__
