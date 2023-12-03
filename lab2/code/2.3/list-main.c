#include "list.h"

#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

size_t INCREASING_LENGTH_COUNT = 0;
size_t DESCENDING_LENGTH_COUNT = 0;
size_t EQUAL_LENGTH_COUNT = 0;

void execMutexlock(pthread_mutex_t *mutex) {
  if (pthread_mutex_lock(mutex)) {
    printf("pthread_mutex_lock() error: %s \n", strerror(errno));
    abort();
  }
}

void execMutexUnlock(pthread_mutex_t *mutex) {
  if (pthread_mutex_unlock(mutex)) {
    printf("pthread_mutex_unlock() error: %s \n", strerror(errno));
    abort();
  }
}

void *countIncreasingLengthPairs(void *arg) {
  printf("increasing [%d %d %d]\n", getpid(), getppid(), gettid());
  Storage *storage = (Storage *)arg;
  while (1) {
    Node *curr1 = storage->first;
    if (curr1 == NULL || curr1->next == NULL) {
      printf("Ooops storage is empty\n"); // ???
      // continue;
      break; // ???
    }
    Node *curr2;
    Node *tmp;

    volatile int counterPairs = 0;
    while ( != NULL) {
      execMutexlock(&(curr->sync));
      execMutexlock(&(curr->next->sync));

      size_t currLength = strlen(curr->value);

      execMutexUnlock(&(curr->next->sync));
      execMutexUnlock(&(curr->sync));

      if (prevLength < currLength) {
        counterPairs++;
      }

      curr = curr->next;
      prevLength = currLength;
    }

    printf("amount of pairs of strings in increaing length: %d",
           counterPairs);

    INCREASING_LENGTH_COUNT++;
  }
  return NULL;
}

void *countDecreasingLengthPairs(void *arg) {
  Storage *storage = (Storage *)arg;
  while (1) {
    Node *curr = storage->first;
    while (curr != NULL && curr->next != NULL) {
      execMutexlock(&(curr->sync));
      execMutexlock(&(curr->next->sync));

      if (strlen(curr->value) > strlen(curr->next->value)) {
        DESCENDING_LENGTH_COUNT++;
      }

      execMutexUnlock(&(curr->next->sync));
      execMutexUnlock(&(curr->sync));
      curr = curr->next;
    }
  }

  return NULL;
}

void *countEqualLengthPaits(void *arg) {
  Storage *storage = (Storage *)arg;
  while (1) {
    Node *curr = storage->first;
    while (curr != NULL && curr->next != NULL) {
      execMutexlock(&(curr->sync));
      execMutexlock(&(curr->next->sync));

      if (strlen(curr->value) == strlen(curr->next->value)) {
        EQUAL_LENGTH_COUNT++;
      }

      execMutexUnlock(&(curr->next->sync));
      execMutexUnlock(&(curr->sync));
      curr = curr->next;
    }
  }

  return NULL;
}

// void *swapElementsOfList(void *arg) {}

char *generateRandomString(char *randomStr) {
  static char *charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOP"
                         "QRSTUVWXYZ0123456789";
  size_t randomLengthStr =
      (1.0 + 100.0 * rand() / RAND_MAX); // nonzero

  for (size_t i = 0; i < randomLengthStr - 1; i++) {
    size_t randSymb = rand() % (sizeof(charset) - 1) % 5;
    randomStr[i] = charset[randSymb];
  }
  randomStr[randomLengthStr] = '\0';

  return randomStr;
}

void generateValuesInStorage(Storage *storage) {
  for (size_t i = 0; i < storage->capacity; i++) {
    char *randomStr = malloc(MAX_STRING_LENGTH * sizeof(char *));
    if (!randomStr) {
      printf("generateRandomString() err: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
    generateRandomString(randomStr);
    // printf("idx: %ld, str: '%s'\n", i, randomStr);

    appendNewNode(storage, randomStr);
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <nodes count>\n", argv[0]);
    return 0;
  }
  int countNodes = atoi(argv[1]);

  srand(time(0));

  Storage *storage = createStorage(countNodes);
  generateValuesInStorage(storage);
  printStorage(storage);

  pthread_t incrementThread;
  // pthread_t decrementThread;
  // pthread_t equalThread;
  // pthread_t swapThread;

  if (pthread_create(&incrementThread, NULL,
                     countIncreasingLengthPairs,
                     (void *)&storage) != 0) {
    printf("incr thread create: %s", strerror(errno));
    return -1;
  }

  // printf("INCREASING: %ld\n", INCREASING_LENGTH_COUNT);

  // printf("OOOAOOAOAOAOA\n");

  // if (pthread_create(&decrementThread, NULL,
  //                    countDecreasingLengthPairs,
  //                    (void *)&storage) != 0) {
  //   printf("decr thread create: %s", strerror(errno));
  //   return -1;
  // }

  // if (pthread_create(&equalThread, NULL, countEqualLengthPaits,
  //                    (void *)&storage) != 0) {
  //   printf("equal thread create: %s", strerror(errno));
  //   return -1;
  // }

  // if (pthread_create(&swapThread, NULL, swapElementsOfList,
  //                    (void *)&storage) != 0) {
  //   printf("swap thread create: %s", strerror(errno));
  //   return -1;
  // }

  // deleteStorage(storage);
  // printf("after deleting\n");
  // printStorage(storage);

  return 0;
}
