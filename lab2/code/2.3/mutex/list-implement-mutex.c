#include "list-mutex.h"
#include "stuff.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Storage *createStorage(int capacity) {
  Storage *storage = malloc(sizeof(Storage));
  if (!storage) {
    printf("createStrorage err: %s", strerror(errno));
    abort();
  }
  storage->capacity = capacity;
  storage->first = NULL;
  return storage;
}

void initMutex(Storage *storage, Node *node) {
  int errMutexAttrInit = pthread_mutexattr_init(&node->mutexAttr);
  if (errMutexAttrInit) {
    printf("mutex attributes init err: %s", strerror(errno));
    destroyStorage(storage);
    abort();
  }

  int errSetType = pthread_mutexattr_settype(
      &node->mutexAttr, PTHREAD_MUTEX_ERRORCHECK);
  if (errSetType) {
    printf("mutex set type err: %s", strerror(errno));
    destroyStorage(storage);
    abort();
  }

  int errMutexInit =
      pthread_mutex_init(&node->mutexSync, &node->mutexAttr);
  if (errMutexInit) {
    printf("mutex init err: %s", strerror(errno));
    destroyStorage(storage);
    abort();
  }
}

void appendNewNode(Storage *storage, const char *value) {
  Node *newNode = (Node *)malloc(sizeof(Node));
  if (!newNode) {
    printf("appendNewNode err: %s", strerror(errno));
    destroyStorage(storage);
    abort();
  }

  newNode->next = storage->first;
  storage->first = newNode;

  strncpy(newNode->value, value, MAX_STRING_LENGTH);
  newNode->value[MAX_STRING_LENGTH - 1] = '\0';

  initMutex(storage, newNode);
}

void generateValuesInStorage(Storage *storage) {
  for (int i = 0; i < storage->capacity; ++i) {
    char *randomStr = malloc(MAX_STRING_LENGTH * sizeof(char *));
    if (!randomStr) {
      printf("generateValuesInStorage() err: %s", strerror(errno));
      destroyStorage(storage);
      abort();
    }
    generateRandomString(randomStr);
    // printf("idx: %d, str: '%s'\n", i, randomStr);

    appendNewNode(storage, randomStr);
  }
}

void destroyStorage(Storage *storage) {
  Node *currentNode = storage->first;
  while (currentNode != NULL) {
    Node *nextNode = currentNode->next;
    pthread_mutexattr_destroy(&currentNode->mutexAttr);
    pthread_mutex_destroy(&currentNode->mutexSync);
    free(currentNode);
    currentNode = nextNode;
  }
  free(storage);
}

void printStorage(Storage *storage) {
  Node *current = storage->first;
  while (current != NULL) {
    printf("%s\n", current->value);
    current = current->next;
  }
  printf("\n");
}
