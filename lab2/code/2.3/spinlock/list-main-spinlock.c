#include "list-spinlock.h"
#include "stuff.h"

#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

void countPairsWithCompare(Storage *storage,
                           int (*compare)(const char *, const char *),
                           volatile int *iterationsCount,
                           volatile int *compareCount) {
  if (storage->first == NULL || storage->first->next == NULL) {
    printf("Not enough elems in storage\n");
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
          if (compare(curr->value, curr2->value)) {
            amountPair++;
          }
          tmp = curr;
          curr = curr->next;

          destroySpinlock(&tmp->sync);
          destroySpinlock(&curr->sync);

          *compareCount += amountPair;
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
    (*iterationsCount)++;
  }
}

void *countIncreasingLengthPairs(void *data) {
  Storage *storage = (Storage *)data;
  countPairsWithCompare(storage, &increasingLengthCompare,
                        &INCREASING_ITERATIONS_COUNT,
                        &INCREASING_COMPARE_COUNT);
  return NULL;
}

void *countDecreasingLengthPairs(void *data) {
  Storage *storage = (Storage *)data;
  countPairsWithCompare(storage, &decreasingLengthCompare,
                        &DECREASING_ITERATIONS_COUNT,
                        &DECREASING_COMPARE_COUNT);
  return NULL;
}

void *countEqualLengthPairs(void *data) {
  Storage *storage = (Storage *)data;
  countPairsWithCompare(storage, &equalLengthCompare,
                        &EQUAL_ITERATIONS_COUNT,
                        &EQUAL_COMPARE_COUNT);
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

int createThreads(ThreadInfo *threads, int numThreads,
                  Storage *storage) {
  for (int i = 0; i < numThreads; i++) {
    if (pthread_create(&threads[i].thread, NULL,
                       threads[i].startRoutine,
                       threads[i].arg) != 0) {
      printf("Thread %d create error: %s", i, strerror(errno));
      return -1;
    }
  }

  return 0;
}

int joinThreads(ThreadInfo *threads, int numThreads,
                Storage *storage) {
  for (int i = 0; i < numThreads; i++) {
    if (pthread_join(threads[i].thread, NULL) != 0) {
      printf("Thread %d join error: %s", i, strerror(errno));
      destroyStorage(storage);
      return -1;
    }
  }

  return 0;
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

  ThreadInfo incrementThread = {
      .startRoutine = countIncreasingLengthPairs, .arg = storage};
  ThreadInfo decrementThread = {
      .startRoutine = countDecreasingLengthPairs, .arg = storage};
  ThreadInfo equalThread = {.startRoutine = countEqualLengthPairs,
                            .arg = storage};
  ThreadInfo swapThread1 = {.startRoutine = countSwapPermutations,
                            .arg = storage};
  ThreadInfo swapThread2 = {.startRoutine = countSwapPermutations,
                            .arg = storage};
  ThreadInfo swapThread3 = {.startRoutine = countSwapPermutations,
                            .arg = storage};
  ThreadInfo displayThread = {.startRoutine = countMonitor,
                              .arg = storage};

  ThreadInfo threads[] = {
      incrementThread, decrementThread, equalThread,  swapThread1,
      swapThread2,     swapThread3,     displayThread};

  int numThreads = sizeof(threads) / sizeof(threads[0]);

  if (createThreads(threads, numThreads, storage) != 0) {
    printf("error in creating threads\n");
    return -1;
  }

  if (joinThreads(threads, numThreads, storage) != 0) {
    printf("error in joining threads\n");
    return -1;
  }

  destroyStorage(storage);

  return 0;
}
