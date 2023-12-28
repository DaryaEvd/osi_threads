#include "list-mutex.h"
#include "stuff.h"

#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SWAPS_AMOUNT 3
#define SWAP1 0
#define SWAP2 1
#define SWAP3 2

#define COEFF_OF_SWAPPING 8

int INCREASING_LENGTH_COUNT = 0;
int DECREASING_LENGTH_COUNT = 0;
int EQUAL_LENGTH_COUNT = 0;
int SWAP_PERMUTATIONS_COUNT = 0;

void execMutexlock(pthread_mutex_t *mutex, Storage *storage) {
  if (pthread_mutex_lock(mutex)) {
    printf("pthread_mutex_lock() error: %s \n", strerror(errno));
    destroyStorage(storage);
    abort();
  }
}

void execMutexUnlock(pthread_mutex_t *mutex, Storage *storage) {
  if (pthread_mutex_unlock(mutex)) {
    printf("pthread_mutex_unlock() error: %s \n", strerror(errno));
    destroyStorage(storage);
    abort();
  }
}

void countPairs(Storage *storage,
                int (*compare)(const char *, const char *)) {
  if (storage->first == NULL || storage->first->next == NULL) {
    printf("Not enough elems in storage to compare\n");
    return;
  }
  while (1) {
    Node *curr = storage->first;
    Node *curr2;
    Node *tmp;
    while (1) {
      if (curr != NULL) {
        execMutexlock(&curr->sync, storage);
        if (curr->next != NULL) {
          execMutexlock(&curr->next->sync, storage);
          volatile int amountPair = 0;
          curr2 = curr->next;

          if (compare(curr->value, curr2->value) == 0) {
            amountPair++;
          }

          tmp = curr;
          curr = curr->next;

          execMutexUnlock(&tmp->sync, storage);
          execMutexUnlock(&curr->sync, storage);

        } else {
          tmp = curr;
          curr = curr->next;

          execMutexUnlock(&tmp->sync, storage);
        }
      } else if (curr == NULL) {
        break;
      } else {
        curr = curr->next;
      }
    }
    if (compare == &increasingLengthCompare) {
      INCREASING_LENGTH_COUNT++;
    } else if (compare == &decreasingLengthCompare) {
      DECREASING_LENGTH_COUNT++;
    } else { // if (compare == &equalLengthCompare)
      EQUAL_LENGTH_COUNT++;
    }
  }
}

void *countIncreasingLengthPairs(void *data) {
  Storage *storage = (Storage *)data;
  countPairs(storage, &increasingLengthCompare);
  return NULL;
}

void *countDecreasingLengthPairs(void *data) {
  Storage *storage = (Storage *)data;
  countPairs(storage, &decreasingLengthCompare);
  return NULL;
}

void *countEqualLengthPairs(void *data) {
  Storage *storage = (Storage *)data;
  countPairs(storage, &equalLengthCompare);
  return NULL;
}

void *countSwapPermutations(void *data) {

  Storage *storage = (Storage *)data;

  while (1) {
    Node *curr1 = storage->first;
    if (curr1 == NULL || curr1->next == NULL ||
        curr1->next->next == NULL) {
      printf("Not enough elems in storage to swap permuts\n");
      break;
    }
    Node *curr2;
    Node *curr3;
    Node *tmp;
    while (1) {
      int willSwap = (rand() % COEFF_OF_SWAPPING == 0);

      if (willSwap) {
        if (curr1 != NULL) {
          execMutexlock(&curr1->sync, storage);
          if (curr1->next != NULL) {
            execMutexlock(&curr1->next->sync, storage);
            if (curr1->next->next != NULL) {
              execMutexlock(&curr1->next->next->sync, storage);
              curr2 = curr1->next;
              curr3 = curr1->next->next;
              {
                curr2->next = curr3->next;
                curr3->next = curr2;
                curr1->next = curr3;
              }
              tmp = curr1;
              curr1 = tmp->next;
              curr2 = curr1->next;

              SWAP_PERMUTATIONS_COUNT++;

              execMutexUnlock(&tmp->sync, storage);
              execMutexUnlock(&curr1->sync, storage);
              execMutexUnlock(&curr2->sync, storage);

            } else {
              tmp = curr1;
              curr1 = curr1->next;

              execMutexUnlock(&tmp->sync, storage);
              execMutexUnlock(&curr1->sync, storage);
            }
          } else {
            tmp = curr1;
            curr1 = curr1->next;

            execMutexUnlock(&tmp->sync, storage);
          }
        } else if (curr1 == NULL) {
          break;
        }
      }
      // if won' swap
      else {
        if (curr1 != NULL) {
          execMutexlock(&curr1->sync, storage);
          if (curr1->next != NULL) {
            execMutexlock(&curr1->next->sync, storage);
            curr2 = curr1;

            execMutexUnlock(&curr1->next->sync, storage);
            execMutexUnlock(&curr1->sync, storage);
          } else {
            execMutexUnlock(&curr1->sync, storage);
          }
        } else {
          break;
        }
      }
    }
  }
  return NULL;
}

void *countMonitor(void *arg) {
  while (1) {
    printf("incr: %d, decr: %d, equal: %d, swap: %d \n",
           INCREASING_LENGTH_COUNT, DECREASING_LENGTH_COUNT,
           EQUAL_LENGTH_COUNT, SWAP_PERMUTATIONS_COUNT);

    sleep(1);
  }
  return NULL;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <nodes count>\n", argv[0]);
    return 0;
  }

  for (int i = 0; argv[1][i] != '\0'; i++) {
    if (!isdigit(argv[1][i])) {
      printf("%s is not a valid number\n", argv[1]);
      return -1;
    }
  }

  int countNodes = atoi(argv[1]);

  srand(time(0));

  Storage *storage = createStorage(countNodes);
  generateValuesInStorage(storage);
  // printStorage(storage);

  pthread_t incrementThread;
  pthread_t decrementThread;
  pthread_t equalThread;
  if (pthread_create(&incrementThread, NULL,
                     countIncreasingLengthPairs,
                     (void *)storage) != 0) {
    printf("incr thread create: %s", strerror(errno));
    free(storage);
    return -1;
  }

  if (pthread_create(&decrementThread, NULL,
                     countDecreasingLengthPairs,
                     (void *)storage) != 0) {
    printf("decr thread create: %s", strerror(errno));
    free(storage);
    return -1;
  }

  if (pthread_create(&equalThread, NULL, countEqualLengthPairs,
                     (void *)storage) != 0) {
    printf("equal thread create: %s", strerror(errno));
    free(storage);
    return -1;
  }

  pthread_t swapThread1;
  pthread_t swapThread2;
  pthread_t swapThread3;

  if (pthread_create(&swapThread1, NULL, countSwapPermutations,
                     (void *)storage) != 0) {
    printf("SWAP_PERMUTATIONS_COUNT 1 thread create: %s",
           strerror(errno));
    free(storage);
    return -1;
  }

  if (pthread_create(&swapThread2, NULL, countSwapPermutations,
                     (void *)storage) != 0) {
    printf("SWAP_PERMUTATIONS_COUNT 2 thread create: %s",
           strerror(errno));
    free(storage);
    return -1;
  }

  if (pthread_create(&swapThread3, NULL, countSwapPermutations,
                     (void *)storage) != 0) {
    printf("SWAP_PERMUTATIONS_COUNT 3 thread create: %s",
           strerror(errno));
    free(storage);
    return -1;
  }

  pthread_t display;
  if (pthread_create(&display, NULL, countMonitor,
                     (void *)&storage) != 0) {
    printf("display thread create: %s", strerror(errno));
    free(storage);
    return -1;
  }

  if (pthread_join(incrementThread, NULL) != 0) {
    printf("incr thread join err: %s", strerror(errno));
    destroyStorage(storage);
    return -1;
  }

  if (pthread_join(decrementThread, NULL) != 0) {
    printf("decr thread join err: %s", strerror(errno));
    destroyStorage(storage);
    return -1;
  }

  if (pthread_join(equalThread, NULL) != 0) {
    printf("equal thread join err: %s", strerror(errno));
    destroyStorage(storage);
    return -1;
  }

  if (pthread_join(swapThread1, NULL) != 0) {
    printf("swap thread1 join err: %s", strerror(errno));
    destroyStorage(storage);
    return -1;
  }

  if (pthread_join(swapThread2, NULL) != 0) {
    printf("swap thread2 join err: %s", strerror(errno));
    destroyStorage(storage);
    return -1;
  }

  if (pthread_join(swapThread3, NULL) != 0) {
    printf("swap thread3 join err: %s", strerror(errno));
    destroyStorage(storage);
    return -1;
  }

  if (pthread_join(display, NULL) != 0) {
    printf("diplay thread join err: %s", strerror(errno));
    destroyStorage(storage);
    return -1;
  }

  destroyStorage(storage);

  return 0;
}
