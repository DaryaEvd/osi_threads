#define _GNU_SOURCE
#include <assert.h>
#include <pthread.h>

#include "queue-spinlock-impl.h"

void createSpinlock(queue_t *q) {
  if (pthread_spin_lock(&q->lock)) {
    printf("pthread_spin_lock() error\n");
  }
}

void destroySpinlock(queue_t *q) {
  if (pthread_spin_unlock(&q->lock)) {
    printf("pthread_spin_unlock() error\n");
  }
}

void *qmonitor(void *arg) {
  queue_t *q = (queue_t *)arg;

  printf("qmonitor: [%d %d %d]\n", getpid(), getppid(), gettid());

  while (1) {
    queue_print_stats(q);
    sleep(1);
  }

  return NULL;
}

queue_t *queue_init(int max_count) {
  queue_t *q = malloc(sizeof(queue_t)); // malloc mem for structure
  if (!q) {
    printf("Cannot allocate memory for a queue\n");
    free(q);
    abort();
  }

  q->first = NULL;
  q->last = NULL;
  q->max_count = max_count;
  q->count = 0;

  q->add_attempts = q->get_attempts = 0;
  q->add_count = q->get_count = 0;

  int errSpintInit =
      pthread_spin_init(&q->lock, PTHREAD_PROCESS_PRIVATE);
  /* про PTHREAD_PROCESS_PRIVATE
  разрешает использовать спин-блокировку только потокам, созданным в
  том же процессе, что и поток, который инициализировал
  спин-блокировку. Если потоки разных процессов пытаются использовать
  такую ​​спин-блокировку, поведение
  не определено. Значение по умолчанию атрибута общего доступа к
  процессу — PTHREAD_PROCESS_PRIVATE.
  */
  if (errSpintInit) {
    printf("queue_init: pthread_spin_init() failed: %s\n",
           strerror(errSpintInit));
    free(q);
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
    free(q);
    abort();
  }

  return q; // return a ptr to queue
}

void queue_destroy(queue_t *q) {
  const int errCancel = pthread_cancel(q->qmonitor_tid);
  if (errCancel) {
    printf("queue_destroy: pthread_cancel() error");
  }

  int errDestroySpin = pthread_spin_destroy(&q->lock);
  if (errDestroySpin) {
    printf("queue_destroy: pthread_spin_destroy() error");
  }

  qnode_t *curr_node = q->first;
  while (curr_node != NULL) {
    qnode_t *tmp = curr_node;
    curr_node = curr_node->next;
    free(tmp);
  }
  free(q);
}

int queue_add(queue_t *q, int val) {
  createSpinlock(q);

  q->add_attempts++; // +1 попытка записать элемент

  assert(q->count <= q->max_count);

  if (q->count == q->max_count) {
    destroySpinlock(q);
    return 0;
  }

  qnode_t *new = malloc(sizeof(qnode_t)); // malloc mem for one node
  if (!new) {
    printf("Cannot allocate memory for new node\n");
    free(new);
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

  destroySpinlock(q);

  return 1;
}

int queue_get(queue_t *q, int *val) {
  createSpinlock(q);

  assert(q->count >= 0);

  if (q->count == 0) {
    destroySpinlock(q);
    return 0;
  }

  qnode_t *tmp = q->first; // save ptr to the 1st node

  *val = tmp->val;           // take val of the 1st node
  q->first = q->first->next; // now next node is the 1st

  free(tmp);      // destroy the 1st node
  q->count--;     // amount of elems in queue
  q->get_count++; // +1 successful попытка добавления элементов

  destroySpinlock(q);

  return 1;
}

void queue_print_stats(queue_t *q) {
  // here we print amount of attempts и how many of them are lucky
  createSpinlock(q);

  int count = q->count;
  long add_attempts = q->add_attempts;
  long get_attempts = q->get_attempts;
  long add_count = q->add_count;
  long get_count = q->get_count;

  destroySpinlock(q);

  printf("\n");
  printf("queue stats: current size %d;\n", count);
  printf("attempts: (add_attempts: %ld; get_attempts: %ld; "
         "add_attempts - get_attempts: %ld)\n",
         add_attempts, get_attempts, add_attempts - get_attempts);
  printf("counts: (add_count: %ld; get_count: %ld; add_count - "
         "get_count: %ld)\n",
         add_count, get_count, add_count - get_count);
}
