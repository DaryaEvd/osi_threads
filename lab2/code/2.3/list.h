#ifndef LIST_H__
#define LIST_H__

#define _GNU_SOURCE
#include <pthread.h>

#define MAX_STRING_LENGTH 100

typedef struct _Node {
  char value[MAX_STRING_LENGTH];
  struct _Node *next;
  pthread_mutex_t sync;
} Node;

typedef struct _Storage {
  Node *first;
  int capacity;
} Storage;

/* ------------- strorage & node functions ------------- */
Storage *createStorage(int capacity);
void appendNewNode(Storage *storage, const char *val);
void deleteStorage(Storage *storage);

char *generateRandomString(char *);

void printStorage(Storage *storage);

/* ----------------- work with thread ----------------- */
// readers of list, the don't modify the list
void *countIncreasingLengthPairs(void *arg);
void *countDecreasingLengthPairs(void *arg);
void *countEqualLengthPaits(void *arg);

// writer of list, should modify the list
// void *swapElementsOfList(void *arg);

#endif // LIST_H__
