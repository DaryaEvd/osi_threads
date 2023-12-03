#include "list.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Storage *createStorage(int capacity) {
  Storage *storage = (Storage *)malloc(sizeof(Storage));
  if (!storage) {
    printf("createStrorage err: %s", strerror(errno));
    abort();
  }
  storage->capacity = capacity;
  storage->first = NULL;
  return storage;
}

void appendNewNode(Storage *storage, const char *val) {
  Node *newNode = (Node *)malloc(sizeof(Node));
  if (!newNode) {
    printf("addNewNode err: %s", strerror(errno));
    return;
  }

  newNode->next = storage->first;
  storage->first = newNode;

  strncpy(newNode->value, val, MAX_STRING_LENGTH);
  newNode->value[MAX_STRING_LENGTH - 1] = '\0';
}

void deleteStorage(Storage *storage) {
  Node *node = storage->first;
  while (node != NULL) {
    Node *nextNode = node->next;
    pthread_mutex_destroy(&(node->sync));
    free(node);
    node = nextNode;
  }
  free(storage);
}

void printStorage(Storage *storage) {
  Node *curr = storage->first;
  while (curr != NULL) {
    printf("%s\n", curr->value);
    curr = curr->next;
  }
}