#define _GNU_SOURCE

#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#include "queue-mutex-impl.h"

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
    queue_print_stats(q);
    usleep(1000);
  }
}

queue_t *queue_init(int max_count) {
  queue_t *q = malloc(sizeof(queue_t)); // malloc mem for structure
  if (!q) {
    printf("Cannot allocate memory for a queue\n");
    abort();
  }

  q->first = NULL;
  q->last = NULL;
  q->max_count = max_count;
  q->count = 0;

  q->add_attempts = q->get_attempts = 0;
  q->add_count = q->get_count = 0;

  int errMutexInit = pthread_mutex_init(&q->mutex, NULL);
  if (errMutexInit) {
    printf("queue_init: pthread_mutex_init() failed: %s\n",
           strerror(errMutexInit));
    abort();
  }

  /*
    we create a thread, save it's thread_id in queue
    and start qmonitor with arg q (which is our queue)
  */
  int err = pthread_create(&q->qmonitor_tid, NULL, qmonitor, q);
  if (err) {
    printf("queue_init: pthread_create() failed: %s\n",
           strerror(err));
    abort();
  }

  return q; // return a ptr to queue
}

void queue_destroy(queue_t *q) {
  int err = pthread_cancel(q->qmonitor_tid);
  if (err) {
    printf("queue_destroy(): pthread_cancel() failed: %s\n",
           strerror(err));
  }

  if (pthread_mutex_destroy(&q->mutex)) {
    printf("queue_destroy: pthread_mutex_destroy() error");
  }

  qnode_t *cur = q->first;
  while (cur != NULL) {
    qnode_t *next = cur->next;
    free(cur);
    cur = next;
  }

  free(q);
}

int queue_add(queue_t *q, int val) {
  if (pthread_mutex_lock(&q->mutex)) {
    printf("pthread_mutex_lock() error\n");
  }
  q->add_attempts++; // +1 попытка записать элемент

  assert(q->count <= q->max_count);

  if (q->count == q->max_count) {
    if (pthread_mutex_unlock(&q->mutex)) {
      printf("pthread_mutex_unlock() error\n");
    }
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
  q->add_count++; // сколько добавили элементов

  if (pthread_mutex_unlock(&q->mutex)) {
    printf("pthread_mutex_unlock() error\n");
  }
  return 1;
}

int queue_get(queue_t *q, int *val) {
  if (pthread_mutex_lock(&q->mutex)) {
    printf("pthread_mutex_lock() error\n");
  }
  q->get_attempts++; // +1 попытка достать элемент
  assert(q->count >= 0);
  if (q->count == 0) {
    if (pthread_mutex_unlock(&q->mutex)) {
      printf("pthread_mutex_unlock() error\n");
    }
    return 0;
  }

  qnode_t *tmp = q->first; // save ptr to the 1st node

  *val = tmp->val;           // take val of the 1st node
  q->first = q->first->next; // now next node is the 1st

  free(tmp);      // delete the 1st node
  q->count--;     // amount of elems in queue
  q->get_count++; // +1 successful попытка добавления элементов

  if (pthread_mutex_unlock(&q->mutex)) {
    printf("pthread_mutex_unlock() error\n");
  }

  return 1;
}

void queue_print_stats(queue_t *q) {
  // here we print amount of attempts и how many of them are lucky
  if (pthread_mutex_lock(&q->mutex)) {
    printf("pthread_mutex_lock() error\n");
  }
  const int count = q->count;
  const long add_attempts = q->add_attempts;
  const long get_attempts = q->get_attempts;
  const long add_count = q->add_count;
  const long get_count = q->get_count;

  if (pthread_mutex_unlock(&q->mutex)) {
    printf("pthread_mutex_unlock() error\n");
  }

  printf("queue stats: current size %d; attempts: (%ld %ld %ld); "
         "counts (%ld %ld %ld)\n",
         count, add_attempts, get_attempts,
         add_attempts - get_attempts, add_count, get_count,
         add_count - get_count);
}
