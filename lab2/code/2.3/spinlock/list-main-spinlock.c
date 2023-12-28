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

int INCREASING_COMPARE_COUNT = 0;
int DECREASING_COMPARE_COUNT = 0;
int EQUAL_COMPARE_COUNT = 0;

int SWAP_PERMUTATIONS_COUNT = 0;

int INCREASING_ITERATIONS_COUNT = 0;
int DECREASING_ITERATIONS_COUNT = 0;
int EQUAL_ITERATIONS_COUNT = 0;

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

int countPairs(Storage *storage,
               int (*compare)(const char *, const char *)) {
  int pairCount = 0;
  if (storage->first == NULL || storage->first->next == NULL) {
    printf("Not enough elems in storage\n");
    return pairCount;
  }

  Node *curr = storage->first;
  Node *curr2;
  while (curr != NULL && curr->next != NULL) {
    createSpinlock(&curr->sync);
    createSpinlock(&curr->next->sync);

    curr2 = curr->next;
    if (compare(curr->value, curr2->value)) {
      pairCount++;
    }

    Node *tmp = curr;
    curr = curr->next;

    destroySpinlock(&tmp->sync);
    destroySpinlock(&curr2->sync);
  }

  return pairCount;
}

void *countIncreasingLengthPairs(void *data) {
  Storage *storage = (Storage *)data;
  while (1) {
    int pairCount = countPairs(storage, &increasingLengthCompare);
    INCREASING_ITERATIONS_COUNT++;
    INCREASING_COMPARE_COUNT += pairCount;
  }
  return NULL;
}

void *countDecreasingLengthPairs(void *data) {
  Storage *storage = (Storage *)data;
  while (1) {
    int pairCount = countPairs(storage, &decreasingLengthCompare);
    DECREASING_ITERATIONS_COUNT++;
    DECREASING_COMPARE_COUNT += pairCount;
  }
  return NULL;
}

void *countEqualLengthPairs(void *data) {
  Storage *storage = (Storage *)data;
  while (1) {
    int pairCount = countPairs(storage, &equalLengthCompare);
    EQUAL_ITERATIONS_COUNT++;
    EQUAL_COMPARE_COUNT += pairCount;
  }
  return NULL;
}

void *countSwapPermutations(void *data) {
  Storage *storage = (Storage *)data;

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
              }
              tmp = curr1;
              curr1 = tmp->next;
              curr2 = curr1->next;

              SWAP_PERMUTATIONS_COUNT++;

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
        } else {
          break;
        }
      }
      // if won't swap
      else {
        if (curr1 != NULL) {
          createSpinlock(&curr1->sync);
          if (curr1->next != NULL) {
            createSpinlock(&curr1->next->sync);
            curr2 = curr1;

            destroySpinlock(&curr1->next->sync);
            destroySpinlock(&curr1->sync);
          } else {
            destroySpinlock(&curr1->sync);
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
    printf("||--- incr: %d, decr: %d, equal: %d, swap: %d \n",
           INCREASING_COMPARE_COUNT, DECREASING_COMPARE_COUNT,
           EQUAL_COMPARE_COUNT, SWAP_PERMUTATIONS_COUNT);
    printf("   iters --- incr iter: %d, decr iter: %d, equal iter: "
           "%d \n ",
           INCREASING_ITERATIONS_COUNT, DECREASING_ITERATIONS_COUNT,
           EQUAL_ITERATIONS_COUNT);
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
  if (pthread_create(&display, NULL, countMonitor, (void *)storage) !=
      0) {
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
