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
    abort();
  }

  if (storage->first != NULL) {
    Node *node = storage->first;
    while (node->next != NULL) {
      node = node->next;
    }
    node->next = newNode;
  } else {
    storage->first = newNode;
  }

  strncpy(newNode->value, val, MAX_STRING_LENGTH);
  newNode->value[MAX_STRING_LENGTH - 1] = '\0';
  newNode->next = NULL;
  pthread_mutex_init(&(newNode->sync), NULL);
}

void printStorage(Storage *storage) {
  Node *curr = storage->first;
  while (curr != NULL) {
    printf("%s\n", curr->value);
    curr = curr->next;
  }
}

// void deleteStorage(Storage *storage) {
//   if (storage->first == NULL) {
//     return;
//   }

//   Node *currNode = storage->first;

//   for(int i = 0; i < storage->capacity; i++) {
//     currNode = currNode->next;
//   }

//   Node *toDelete = currNode->next;
//   currNode->next = toDelete->next;

//   free(toDelete);

//   pthread_mutex_destroy(&currNode->sync);
//   free(storage);
// }

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
