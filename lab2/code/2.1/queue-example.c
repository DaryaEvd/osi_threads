#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "queue.h"

int main() {
  queue_t *q;

  printf("main: [%d %d %d]\n", getpid(), getppid(), gettid());

  q = queueInit(1000);

  // здесь добавляем значение в очередь
  for (int i = 0; i < 10; i++) {
    int ok = queueAdd(q, i);

    printf("ok %d: add value %d\n", ok, i);

    queuePrintStats(q);
  }

  // здесь достаем это значение из очереди
  for (int i = 0; i < 12; i++) {
    int val = -1;
    int ok = queueGet(q, &val);

    printf("ok: %d: get value %d\n", ok, val);

    queuePrintStats(q);
  }

  queueDestroy(q);

  return 0;
}
