#define _GNU_SOURCE
#include <assert.h>
#include <pthread.h>

#include "queue.h"

void *qmonitor(void *arg) {
  queueT *q = (queueT *)arg;

  printf("qmonitor: [%d %d %d]\n", getpid(), getppid(), gettid());

  while (1) {
    queuePrintStats(q);
    sleep(1);
  }

  return NULL;
}

queueT *queueInit(int maxCount) {
  queueT *q = malloc(sizeof(queueT)); // malloc mem for structure
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

  /*
  we create a thread, save it's thread_id in queue
  and start qmonitor with arg q (which is our queue)
  */
  int err = pthread_create(&q->qmonitorTid, NULL, qmonitor, q);
  if (err) {
    printf("queueInit: pthread_create() failed: %s\n", strerror(err));
    free(q);
    abort();
  }

  return q; // return a ptr to queue
}

void queueDestroy(queueT *q) {
  const int errCancel = pthread_cancel(q->qmonitorTid);
  if (errCancel) {
    printf("pthread_cancel() error %s", strerror(errno));
  }

  qnodeT *currNode = q->first;
  while (currNode != NULL) {
    qnodeT *tmp = currNode;
    currNode = currNode->next;
    free(tmp);
  }
  free(q);
}

int queueAdd(queueT *q, int val) {
  q->addAttempts++; // +1 попытка записать элемент

  assert(q->count <= q->maxCount);

  if (q->count == q->maxCount) {
    return 0;
  }

  qnodeT *new = malloc(sizeof(qnodeT)); // malloc mem for one node
  if (!new) {
    printf("Cannot allocate memory for new node\n");
    queueDestroy(q);
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

  return 1;
}

int queueGet(queueT *q, int *val) {
  q->getAttempts++; // +1 попытка достать элемент

  assert(q->count >= 0);

  if (q->count == 0) {
    return 0;
  }

  qnodeT *tmp = q->first; // save ptr to the 1st node

  *val = tmp->val;           // take val of the 1st node
  q->first = q->first->next; // now next node is the 1st

  free(tmp);     // delete the 1st node
  q->count--;    // amount of elems in queue
  q->getCount++; // +1 successful попытка добавления элементов

  return 1;
}

void queuePrintStats(queueT *q) {
  /*
  here we print amount of попыток и how many of them are удачные
  */
  printf("\n");
  printf("queue stats: current size %d;\n", q->count);
  printf("attempts: (addAttempts: %ld; getAttempts: %ld; "
         "addAttempts - getAttempts: %ld)\n",
         q->addAttempts, q->getAttempts,
         q->addAttempts - q->getAttempts);
  printf("counts: (addCount: %ld; getCount: %ld; addCount - "
         "getCount: %ld)\n",
         q->addCount, q->getCount, q->addCount - q->getCount);
}
