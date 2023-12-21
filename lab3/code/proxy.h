#ifndef PROXY_H__
#define PROXY_H__

int execHttpProxy(const int port);

int parseHttpRequest(char *buffer, int bufferLength, char *ip,
                     int lengthIP, char *port);
void *handleRequestConnection(void *args);

#endif // PROXY_H__
