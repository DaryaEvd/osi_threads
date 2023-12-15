#include "list-spinlock.h"
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

void createSpinlock(pthread_spinlock_t *q) {
  if (pthread_spin_lock(q)) {
    printf("pthread_spin_lock() error: %s\n", strerror(errno));
    abort();
  }
}

void destroySpinlock(pthread_spinlock_t *q) {
  if (pthread_spin_unlock(q)) {
    printf("pthread_spin_unlock() error: %s\n", strerror(errno));
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
        createSpinlock(&curr->sync);
        if (curr->next != NULL) {
          createSpinlock(&curr->next->sync);
          volatile int amountPair = 0;
          curr2 = curr->next;

          if (compare(curr->value, curr2->value) == 0) {
            amountPair++;
          }

          tmp = curr;
          curr = curr->next;

          destroySpinlock(&tmp->sync);
          destroySpinlock(&curr->sync);
        } else {
          tmp = curr;
          curr = curr->next;

          destroySpinlock(&tmp->sync);
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
  SwapInfo *swapInfo = (SwapInfo *)data;
  Storage *storage = swapInfo->storage;
  int *counter = swapInfo->swapCounter;

  while (1) {
    Node *curr1 = storage->first;
    if (curr1 == NULL || curr1->next == NULL ||
        curr1->next->next == NULL) {
      printf(
          "Not enough elems in storage to SWAP_PERMUTATIONS_COUNT\n");
      break;
    }
    Node *curr2;
    Node *curr3;
    Node *tmp;
    while (1) {
      int willSwap = (rand() % COEFF_OF_SWAPPING == 0);
      if (willSwap) {
        if (curr1 != NULL) {
          createSpinlock(&curr1->sync);
          if (curr1->next != NULL) {
            createSpinlock(&curr1->next->sync);
            if (curr1->next->next != NULL) {
              createSpinlock(&curr1->next->next->sync);
              curr2 = curr1->next;
              curr3 = curr1->next->next;
              {
                curr2->next = curr3->next;
                curr3->next = curr2;
                curr1->next = curr3;
                (*counter)++;
              }
              tmp = curr1;
              curr1 = tmp->next;
              curr2 = curr1->next;

              destroySpinlock(&tmp->sync);
              destroySpinlock(&curr1->sync);
              destroySpinlock(&curr2->sync);

            } else {
              tmp = curr1;
              curr1 = curr1->next;

              destroySpinlock(&tmp->sync);
              destroySpinlock(&curr1->sync);
            }
          } else {
            tmp = curr1;
            curr1 = curr1->next;

            destroySpinlock(&tmp->sync);
          }
        } else if (curr1 == NULL) {
          break;
        }
      }
      // if won't swap
      else {
        if (curr1 != NULL) {
          createSpinlock(&curr1->sync);
          if (curr1->next != NULL) {
            createSpinlock(&curr1->next->sync);
            curr2 = curr1->next;
            curr2 = curr1;

            destroySpinlock(&curr1->next->sync);
            destroySpinlock(&curr1->sync);
          } else {
            destroySpinlock(&curr1->sync);
          }
        } else {
          break;
        }
        curr1 = curr1->next;
      }
    }
  }
  return NULL;
}

void *countMonitor(void *arg) {
  int *swapCounters = (int *)arg;
  while (1) {
    printf("incr: %d, decr: %d, equal: %d, swap1: %d, swap2: %d, "
           "swap3: "
           "%d\n",
           INCREASING_LENGTH_COUNT, DECREASING_LENGTH_COUNT,
           EQUAL_LENGTH_COUNT, swapCounters[SWAP1],
           swapCounters[SWAP2], swapCounters[SWAP3]);

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
      return -11;
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

  int *swapCounters = calloc(SWAPS_AMOUNT, sizeof(int));
  if (!swapCounters) {
    printf("no mem for calloc(): %s", strerror(errno));
    free(storage);
    return -1;
  }

  SwapInfo swapInfo1 = {storage, &swapCounters[SWAP1]};
  SwapInfo swapInfo2 = {storage, &swapCounters[SWAP2]};
  SwapInfo swapInfo3 = {storage, &swapCounters[SWAP3]};

  if (pthread_create(&swapThread1, NULL, countSwapPermutations,
                     &swapInfo1) != 0) {
    printf("SWAP_PERMUTATIONS_COUNT 1 thread create: %s",
           strerror(errno));
    free(storage);
    return -1;
  }

  if (pthread_create(&swapThread2, NULL, countSwapPermutations,
                     &swapInfo2) != 0) {
    printf("SWAP_PERMUTATIONS_COUNT 2 thread create: %s",
           strerror(errno));
    free(storage);
    return -1;
  }

  if (pthread_create(&swapThread3, NULL, countSwapPermutations,
                     &swapInfo3) != 0) {
    printf("SWAP_PERMUTATIONS_COUNT 3 thread create: %s",
           strerror(errno));
    free(storage);
    return -1;
  }

  pthread_t display;
  if (pthread_create(&display, NULL, countMonitor, swapCounters) !=
      0) {
    printf("display thread create: %s", strerror(errno));
    free(storage);
    return -1;
  }

  if (pthread_join(incrementThread, NULL) != 0) {
    printf("incr thread join err: %s", strerror(errno));
    free(storage);
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
