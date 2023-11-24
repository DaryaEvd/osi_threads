#include "list.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Storage *createStorage() {
  Storage *storage = (Storage *)malloc(sizeof(Storage));
  if (!storage) {
    printf("createStrorage err: %s", strerror(errno));
    abort();
  }
  storage->first = NULL;
  return storage;
}

void appendNewNode(Storage *storage, const char *val) {
  Node *newNode = (Node *)malloc(sizeof(Node));
  if (!newNode) {
    printf("addNewNode err: %s", strerror(errno));
    abort();
  }

  strncpy(newNode->value, val, MAX_STRING_LENGTH);
  newNode->value[MAX_STRING_LENGTH - 1] = '\0';

  pthread_mutex_init(&(newNode->sync), NULL);

  storage->first = newNode;
}

void deleteStorage(Storage *storage, Node *node) {
  if (storage->first == NULL || node == NULL) {
    return;
  }

  if (storage->first == node) {
    storage->first = storage->first;
  } else {
    Node *currNode = storage->first;

    while (currNode != NULL) {
      Node *tmp = currNode;
      currNode = currNode->next;
      free(tmp);
    }
    free(node);
  }

  pthread_mutex_destroy(&node->sync);
  free(node);
  free(storage);
}
