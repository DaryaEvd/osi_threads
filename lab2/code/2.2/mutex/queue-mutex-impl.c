#define _GNU_SOURCE

#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#include "queue-mutex-impl.h"

void execMutexlock(queue_t *q) {
  if (pthread_mutex_lock(&q->mutex)) {
    printf("pthread_mutex_lock() error: %s \n", strerror(errno));
    abort();
  }
}

void execMutexUnlock(queue_t *q) {
  if (pthread_mutex_unlock(&q->mutex)) {
    printf("pthread_mutex_unlock() error: %s \n", strerror(errno));
    abort();
  }
}

qnode_t *create_node(int val) {
  qnode_t *new = malloc(sizeof(qnode_t));
  if (!new) {
    printf("Cannot allocate memory for new node\n");
    abort();
  }
  new->val = val;
  new->next = NULL;

  return new;
}

void *qmonitor(void *arg) {
  queue_t *q = (queue_t *)arg;

  printf("qmonitor: [%d %d %d]\n", getpid(), getppid(), gettid());

  while (1) {
    queuePrintStats(q);
    sleep(1);
  }
}

queue_t *queueInit(int maxCount) {
  queue_t *q = malloc(sizeof(queue_t)); // malloc mem for structure
  if (!q) {
    printf("Cannot allocate memory for a queue\n");
    abort();
  }

  q->first = NULL;
  q->last = NULL;
  q->maxCount = maxCount;
  q->count = 0;

  q->addAttempts = q->getAttempts = 0;
  q->addCount = q->getCount = 0;

  int errMutexInit = pthread_mutex_init(&q->mutex, NULL);
  if (errMutexInit) {
    printf("queueInit: pthread_mutex_init() failed: %s\n",
           strerror(errMutexInit));
    abort();
  }

  /*
    we create a thread, save it's thread_id in queue
    and start qmonitor with arg q (which is our queue)
  */
  int err = pthread_create(&q->qmonitorTid, NULL, qmonitor, q);
  if (err) {
    printf("queueInit: pthread_create() failed: %s\n",
           strerror(err));
    abort();
  }

  return q; // return a ptr to queue
}

void queueDestroy(queue_t *q) {
  int err = pthread_cancel(q->qmonitorTid);
  if (err) {
    printf("queueDestroy(): pthread_cancel() failed: %s\n",
           strerror(err));
  }

  if (pthread_mutex_destroy(&q->mutex)) {
    printf("queueDestroy: pthread_mutex_destroy() error : %s\n",
           strerror(errno));
  }

  qnode_t *cur = q->first;
  while (cur != NULL) {
    qnode_t *next = cur->next;
    free(cur);
    cur = next;
  }

  free(q);
}

int queueAdd(queue_t *q, int val) {
  execMutexlock(q);

  q->addAttempts++; // +1 попытка записать элемент

  assert(q->count <= q->maxCount);

  if (q->count == q->maxCount) {
    execMutexUnlock(q);
    return 0;
  }

  qnode_t *new = malloc(sizeof(qnode_t)); // malloc mem for one node
  if (!new) {
    printf("Cannot allocate memory for new node\n");
    abort();
  }

  new->val = val;
  new->next = NULL;

  if (!q->first) { // only one 1st node in queue
    q->first = q->last = new;
  } else { // not the 1st node in queue
    q->last->next = new;
    q->last = q->last->next;
  }

  q->count++; // количество элементов на текущий момент
  q->addCount++; // сколько добавили элементов

  execMutexUnlock(q);
  return 1;
}

int queueGet(queue_t *q, int *val) {
  execMutexlock(q);

  q->getAttempts++; // +1 попытка достать элемент
  assert(q->count >= 0);
  if (q->count == 0) {
    execMutexUnlock(q);
    return 0;
  }

  qnode_t *tmp = q->first; // save ptr to the 1st node

  *val = tmp->val;           // take val of the 1st node
  q->first = q->first->next; // now next node is the 1st

  free(tmp);      // delete the 1st node
  q->count--;     // amount of elems in queue
  q->getCount++; // +1 successful попытка добавления элементов

  execMutexUnlock(q);

  return 1;
}

void queuePrintStats(queue_t *q) {
  // here we print amount of attempts и how many of them are lucky
  execMutexlock(q);

  const int count = q->count;
  const long addAttempts = q->addAttempts;
  const long getAttempts = q->getAttempts;
  const long addCount = q->addCount;
  const long getCount = q->getCount;

  execMutexUnlock(q);

  printf("queue stats: current size %d; attempts: (%ld %ld %ld); "
         "counts (%ld %ld %ld)\n",
         count, addAttempts, getAttempts,
         addAttempts - getAttempts, addCount, getCount,
         addCount - getCount);
}
