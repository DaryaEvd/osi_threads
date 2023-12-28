#ifndef LIST_H__
#define LIST_H__

#define _GNU_SOURCE
#include <pthread.h>

#define MAX_STRING_LENGTH 100

typedef struct _Node {
  char value[MAX_STRING_LENGTH];
  struct _Node *next;

  pthread_mutex_t sync;
  pthread_mutexattr_t mutexAttr;
} Node;

typedef struct _Storage {
  Node *first;
  int capacity;
} Storage;

typedef struct _ThreadInfo {
  pthread_t thread;
  void *(*startRoutine)(void *);
  void *arg;
} ThreadInfo;

/* ------------- strorage & node functions ------------- */

Storage *createStorage(int capacity);
void appendNewNode(Storage *storage, const char *value);
void generateValuesInStorage(Storage *storage);
void destroyStorage(Storage *storage);
void printStorage(Storage *storage);

#endif // LIST_H__
