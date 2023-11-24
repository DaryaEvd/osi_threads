#include "list.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void *countIncreasingLengthPairs(void *arg) { return NULL; }
void *countDecreasingLengthPairs(void *arg) { return NULL; }
void *countEqualLengthPaits(void *arg) { return NULL; }

void *swapElementsOfList(void *arg) { return NULL; }

char *generateRandomString() {
  char *randomStr = malloc(MAX_STRING_LENGTH * sizeof(char *));
  if (!randomStr) {
    printf("generateRandomString() err: %s", strerror(errno));
    abort();
    // exit(EXIT_FAILURE);
    // return NULL; // ???
  }

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

int main(int argc, char **argv) {
  srand(time(0));

  // for (int i = 0; i < 5; i++) {
  // printf("%s\n", generateRandomString());
  // }

  Storage storage;
  storage.first = NULL;

  pthread_t incrementThread;
  pthread_t decrementThread;
  pthread_t equalThread;
  pthread_t swapThread;

  if (pthread_create(&incrementThread, NULL,
                     countIncreasingLengthPairs,
                     (void *)&storage) != 0) {
    printf("incr thread create: %s", strerror(errno));
    return -1;
  }

  if (pthread_create(&decrementThread, NULL,
                     countDecreasingLengthPairs,
                     (void *)&storage) != 0) {
    printf("decr thread create: %s", strerror(errno));
    return -1;
  }

  if (pthread_create(&equalThread, NULL, countEqualLengthPaits,
                     (void *)&storage) != 0) {
    printf("equal thread create: %s", strerror(errno));
    return -1;
  }

  if (pthread_create(&swapThread, NULL, swapElementsOfList,
                     (void *)&swapElementsOfList) != 0) {
    printf("swap thread create: %s", strerror(errno));
    return -1;
  }

  return 0;
}
