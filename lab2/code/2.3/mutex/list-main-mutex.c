#include "list-mutex.h"
#include "stuff.h"

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

void *countIncreasingLengthPairs(void *data) {
  Storage *storage = (Storage *)data;

  while (1) {
    Node *curr = storage->first;
    if (curr == NULL || curr->next == NULL) {
      printf("Not enough elems in storage to increase\n");
      break;
    }
    Node *curr2;
    Node *tmp;
    while (1) {
      if (curr != NULL) {
        execMutexlock(&curr->mutexSync, storage);
        if (curr->next != NULL) {
          execMutexlock(&curr->next->mutexSync, storage);
          volatile int amountPairIncrease = 0;
          curr2 = curr->next;
          if (strlen(curr->value) < strlen(curr2->value)) {
            amountPairIncrease++;
          }
          tmp = curr;
          curr = curr->next;

          execMutexUnlock(&tmp->mutexSync, storage);
          execMutexUnlock(&curr->mutexSync, storage);

        } else {
          tmp = curr;
          curr = curr->next;

          execMutexUnlock(&tmp->mutexSync, storage);
        }
      } else if (curr == NULL) {
        break;
      } else {
        curr = curr->next;
      }
    }
    INCREASING_LENGTH_COUNT++;
  }
  return NULL;
}

void *countDecreasingLengthPairs(void *data) {
  Storage *storage = (Storage *)data;
  if (storage->first == NULL || storage->first->next == NULL) {
    printf("Not enough elems in storage to descrease\n");
    return NULL;
  }
  while (1) {
    Node *curr = storage->first;
    Node *curr2;
    Node *tmp;
    while (1) {
      if (curr != NULL) {
        execMutexlock(&curr->mutexSync, storage);

        if (curr->next != NULL) {
          execMutexlock(&curr->next->mutexSync, storage);
          volatile int amountPairDecrease = 0;
          curr2 = curr->next;
          if (strlen(curr->value) == strlen(curr2->value)) {
            amountPairDecrease++;
          }
          tmp = curr;
          curr = curr->next;

          execMutexUnlock(&tmp->mutexSync, storage);
          execMutexUnlock(&curr->mutexSync, storage);
        } else {
          tmp = curr;
          curr = curr->next;

          execMutexUnlock(&tmp->mutexSync, storage);
        }
      } else if (curr == NULL) {
        break;
      } else {
        curr = curr->next;
      }
    }
    DECREASING_LENGTH_COUNT++;
  }
  return NULL;
}

void *countEqualLengthPaits(void *data) {
  Storage *storage = (Storage *)data;
  if (storage->first == NULL || storage->first->next == NULL) {
    printf("Not enough elems in storage to equal\n");
    return NULL;
  }

  while (1) {
    Node *curr = storage->first;
    Node *curr2;
    Node *tmp;
    while (1) {
      if (curr != NULL) {
        execMutexlock(&curr->mutexSync, storage);
        if (curr->next != NULL) {
          execMutexlock(&curr->next->mutexSync, storage);
          volatile int amountPairEqual = 0;
          curr2 = curr->next;
          if (strlen(curr->value) > strlen(curr2->value)) {
            amountPairEqual++;
          }
          tmp = curr;
          curr = curr->next;

          execMutexUnlock(&tmp->mutexSync, storage);
          execMutexUnlock(&curr->mutexSync, storage);
        } else {
          tmp = curr;
          curr = curr->next;

          execMutexUnlock(&tmp->mutexSync, storage);
        }
      } else if (curr == NULL) {
        break;
      } else {
        curr = curr->next;
      }
    }
    EQUAL_LENGTH_COUNT++;
  }
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
      printf("Not enough elems in storage to swap permuts\n");
      break;
    }
    Node *curr2;
    Node *curr3;
    Node *tmp;
    while (1) {
      if (curr1 != NULL) {
        execMutexlock(&curr1->mutexSync, storage);

        if (curr1->next != NULL) {
          execMutexlock(&curr1->next->mutexSync, storage);
          if (curr1->next->next != NULL) {
            execMutexlock(&curr1->next->next->mutexSync, storage);
            curr2 = curr1->next;
            curr3 = curr1->next->next;
            if (rand() % COEFF_OF_SWAPPING == 0) {
              curr2->next = curr3->next;
              curr3->next = curr2;
              curr1->next = curr3;
              (*counter)++;
            }
            tmp = curr1;
            curr1 = tmp->next;
            curr2 = curr1->next;

            execMutexUnlock(&tmp->mutexSync, storage);
            execMutexUnlock(&curr1->mutexSync, storage);
            execMutexUnlock(&curr2->mutexSync, storage);

          } else {
            tmp = curr1;
            curr1 = curr1->next;

            execMutexUnlock(&tmp->mutexSync, storage);
            execMutexUnlock(&curr1->mutexSync, storage);
          }
        } else {
          tmp = curr1;
          curr1 = curr1->next;

          execMutexUnlock(&tmp->mutexSync, storage);
        }
      } else if (curr1 == NULL) {
        break;
      } else {
        curr1 = curr1->next;
      }
    }
  }
  return NULL;
}

void *countMonitor(void *arg) {
  int *swapCounters = (int *)arg;
  while (1) {
    printf(
        "incr: %d, decr: %d, equal: %d, swap1: %d, swap2: %d, swap3: "
        "%d\n",
        INCREASING_LENGTH_COUNT, DECREASING_LENGTH_COUNT,
        EQUAL_LENGTH_COUNT, swapCounters[SWAP1], swapCounters[SWAP2],
        swapCounters[SWAP3]);

    sleep(1);
  }
  return NULL;
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
  // printStorage(storage);

  pthread_t incrementThread;
  pthread_t decrementThread;
  pthread_t equalThread;
  if (pthread_create(&incrementThread, NULL,
                     countIncreasingLengthPairs,
                     (void *)storage) != 0) {
    printf("incr thread create: %s", strerror(errno));
    return -1;
  }

  if (pthread_create(&decrementThread, NULL,
                     countDecreasingLengthPairs,
                     (void *)storage) != 0) {
    printf("decr thread create: %s", strerror(errno));
    return -1;
  }

  if (pthread_create(&equalThread, NULL, countEqualLengthPaits,
                     (void *)storage) != 0) {
    printf("EQUAL_LENGTH_COUNT thread create: %s", strerror(errno));
    return -1;
  }

  pthread_t swapThread1;
  pthread_t swapThread2;
  pthread_t swapThread3;

  int *swapCounters = calloc(SWAPS_AMOUNT, sizeof(int));
  if (!swapCounters) {
    printf("error in mem alloc for swap counter: %s",
           strerror(errno));
    destroyStorage(storage);
    return 0;
  }

  SwapInfo swapInfo1 = {storage, &swapCounters[SWAP1]};
  SwapInfo swapInfo2 = {storage, &swapCounters[SWAP2]};
  SwapInfo swapInfo3 = {storage, &swapCounters[SWAP3]};

  if (pthread_create(&swapThread1, NULL, countSwapPermutations,
                     &swapInfo1) != 0) {
    printf("SWAP_PERMUTATIONS_COUNT 1 thread create: %s",
           strerror(errno));
    return -1;
  }

  if (pthread_create(&swapThread2, NULL, countSwapPermutations,
                     &swapInfo2) != 0) {
    printf("SWAP_PERMUTATIONS_COUNT 2 thread create: %s",
           strerror(errno));
    return -1;
  }

  if (pthread_create(&swapThread3, NULL, countSwapPermutations,
                     &swapInfo3) != 0) {
    printf("SWAP_PERMUTATIONS_COUNT 3 thread create: %s",
           strerror(errno));
    return -1;
  }

  pthread_t display;
  if (pthread_create(&display, NULL, countMonitor, swapCounters) !=
      0) {
    printf("display thread create: %s", strerror(errno));
    return -1;
  }

  pthread_join(incrementThread, NULL);
  pthread_join(decrementThread, NULL);
  pthread_join(equalThread, NULL);
  pthread_join(swapThread1, NULL);
  pthread_join(swapThread2, NULL);
  pthread_join(swapThread3, NULL);
  pthread_join(display, NULL);

  destroyStorage(storage);
  return 0;
}
