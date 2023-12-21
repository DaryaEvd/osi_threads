#ifndef NETWORK_H__
#define NETWORK_H__

#define BUFFER_SIZE 2048

int initSocketListener(int socketListener, int port);

void exchangeData(char *buffer, int hostSocketFD, int clientSocketFD,
                  int writeBytesToHost, int readBytesFromClient);

#endif // NETWORK_H__
