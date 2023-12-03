#ifndef __FITOS_QUEUE_H__
#define __FITOS_QUEUE_H__

#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct _QueueNode {
  int val;
  struct _QueueNode *next;
} qnode_t;

typedef struct _Queue {
  qnode_t *first;
  qnode_t *last;

  pthread_t qmonitorTid;

  int count; // amount of elements in a current moment
  int maxCount;

  // queue statistics
  long addAttempts; // типа сколько попыток было сделано, чтоб
                     // запистать элемент
  long getAttempts; // количество попыток прочитать элемент (не факт,
                     // что смогли прочитать из очереди, тк она может
                     // быть пустой вообще, например)
  long addCount; // сколько из этих попыток было успешных, то есть
                  // вообще сколько добавли элементов
  long getCount; // сколько прочитали элементов

  pthread_mutex_t mutex;

  pthread_cond_t condVar;

  int flagCanShareData;

} queue_t;
/*
 То есть в идеале хотелось бы, чтобы когда мы пытаемся что-то
 добавить, мы могли иметь возможность добавить. А когда хотим что-то
 прочитать, мы имели возможность что-то прочитать
 Для этого мы и сравниваем queue statistics parameters))
 */

queue_t *queueInit(int maxCount);
void queueDestroy(queue_t *q);
int queueAdd(queue_t *q, int val);
int queueGet(queue_t *q, int *val);
void queuePrintStats(queue_t *q);

#endif // __FITOS_QUEUE_H__
