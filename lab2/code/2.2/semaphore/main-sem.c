#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <pthread.h>
#include <sched.h>

#include "queue-sem-impl.h"

#define RED "\033[41m"
#define NOCOLOR "\033[0m"

void set_cpu(int n) {
  int err;
  cpu_set_t cpuset; // data structure to describe CPU mask
  pthread_t tid = pthread_self();

  CPU_ZERO(&cpuset);   // clears set, so that it contains no CPUs.
  CPU_SET(n, &cpuset); // add CPU cpu to set

  //  Limit specified thread TH to run only on the processors
  //  represented in CPUSET
  err = pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
  if (err) {
    printf("set_cpu: pthread_setaffinity failed for cpu %d\n", n);
    return;
  }

  printf("set_cpu: set cpu %d\n", n);
}

// читает чиселки из очереди
void *reader(void *arg) {
  // ожидается последовательная запись чисел, начиная с нуля
  int expected = 0;
  queue_t *q = (queue_t *)arg;
  printf("reader [%d %d %d]\n", getpid(), getppid(), gettid());

  set_cpu(1);

  while (1) {
    int val = -1;
    int ok = queueGet(q, &val);
    if (!ok) {
      continue;
    }

    // проверка на то, что ридер считывает последовательность
    // неотрицательных чисел
    if (expected != val) {
      printf(RED
             "ERROR: get value is '%d' but expected - '%d'" NOCOLOR
             "\n",
             val, expected);
    }
    expected = val + 1;
  }

  return NULL;
}

// последовательно пишет чиселки, начиная с нуля
void *writer(void *arg) {
  int i = 0;
  queue_t *q = (queue_t *)arg;
  printf("writer [%d %d %d]\n", getpid(), getppid(), gettid());

  set_cpu(2);

  while (1) {
    // uncomment usleep() for пункт d
    // usleep(1); // !!!!!!!!!!!!!
    //

    int ok = queueAdd(q, i);
    if (!ok) {
      continue;
    }
    i++;
  }

  return NULL;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <max queue size>\n", argv[0]);
    return -1;
  }
  printf("main [%d %d %d]\n", getpid(), getppid(), gettid());

  const int sizeQueue = atoi(argv[1]);
  queue_t *q = queueInit(sizeQueue);

  pthread_t tidReader;
  int createErr = pthread_create(&tidReader, NULL, reader, q);
  if (createErr) {
    printf("main: pthread_create() failed: %s\n",
           strerror(createErr));
    return -1;
  }

  sched_yield();

  pthread_t tidWriter;
  createErr = pthread_create(&tidWriter, NULL, writer, q);
  if (createErr) {
    printf("main: pthread_create() failed: %s\n",
           strerror(createErr));
    return -1;
  }

  void *retVal;
  int joinError = pthread_join(tidReader, &retVal);
  if (joinError) {
    printf("main: pthread_join() failed: %s\n", strerror(joinError));
    return -1;
  }

  joinError = pthread_join(tidWriter, &retVal);
  if (joinError) {
    printf("main: pthred_join() failed: %s\n", strerror(joinError));
    return -1;
  }

  queueDestroy(q);

  pthread_exit(NULL);

  return 0;
}
