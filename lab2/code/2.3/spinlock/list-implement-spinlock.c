#include "list.h"
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

void appendNewNode(Storage *storage, const char *value) {
  Node *new_node = (Node *)malloc(sizeof(Node));

  if (!new_node) {
    printf("appendNewNode err: %s", strerror(errno));
    abort();
  }

  new_node->next = storage->first;
  storage->first = new_node;

  strncpy(new_node->value, value, MAX_STRING_LENGTH);
  new_node->value[MAX_STRING_LENGTH - 1] = '\0';

  int errSpintInit =
      pthread_spin_init(&new_node->sync, PTHREAD_PROCESS_PRIVATE);
  if (errSpintInit) {
    printf("appendNewNode: pthread_spin_init() failed: %s\n",
           strerror(errSpintInit));
    abort();
  }
}

void generateValuesInStorage(Storage *storage) {
  for (int i = 0; i < storage->capacity; ++i) {
    char *randomStr = malloc(MAX_STRING_LENGTH * sizeof(char *));
    if (!randomStr) {
      printf("generateValuesInStorage() err: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
    generateRandomString(randomStr);
    // printf("idx: %d, str: '%s'\n", i, randomStr);

    appendNewNode(storage, randomStr);
  }
}

void printStorage(Storage *storage) {
  Node *current = storage->first;
  while (current != NULL) {
    printf("%s\n", current->value);
    current = current->next;
  }
  printf("\n");
}
