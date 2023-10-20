#define _GNU_SOURCE
#include <assert.h>
#include <pthread.h>

#include "queue.h"

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
  int err;

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

  /*
  we create a thread, save it's thread id in queue
  and start qmonitor with arg q (which is our queue)
  */
  err = pthread_create(&q->qmonitor_tid, NULL, qmonitor, q);
  if (err) {
    printf("queue_init: pthread_create() failed: %s\n",
           strerror(err));
    abort();
  }

  return q; // return a ptr to queue
}

void queue_destroy(queue_t *q) {
  // TODO: It's needed to implement this function
  qnode_t *curr_node = q->first;
  while (curr_node != NULL) {
    qnode_t *tmp = curr_node;
    curr_node = curr_node->next;
    free(tmp);
  }
  free(q);
}

int queue_add(queue_t *q, int val) {
  q->add_attempts++; // +1 попытка

  assert(q->count <= q->max_count);

  if (q->count == q->max_count)
    return 0;

  qnode_t *new = malloc(sizeof(qnode_t)); // malloc mem for one node
  if (!new) {
    printf("Cannot allocate memory for new node\n");
    abort();
  }

  new->val = val;
  new->next = NULL;

  if (!q->first) // only one 1st node in queue
    q->first = q->last = new;
  else { // not the 1st node in queue
    q->last->next = new;
    q->last = q->last->next;
  }

  q->count++; // количество нод на текущий момент
  q->add_count++; // типа удачная попытка добавить ноду

  return 1;
}

int queue_get(queue_t *q, int *val) {
  q->get_attempts++; // +1 попытка

  assert(q->count >= 0);

  if (q->count == 0)
    return 0;

  qnode_t *tmp = q->first; // save ptr to the 1st node

  *val = tmp->val;           // take val of the 1st node
  q->first = q->first->next; // now next node is the 1st

  free(tmp);      // delete the 1st node
  q->count--;     // amount of elems in queue
  q->get_count++; // +1 successful попытка

  return 1;
}

void queue_print_stats(queue_t *q) {
  /*
  here we print amount of попыток и how many of them are удачные
  */
  printf("\n");
  printf("queue stats: current size %d;\n", q->count);
  printf("attempts: (add_attempts: %ld; get_attempts: %ld; "
         "add_attempts - get_attempts: %ld\n",
         q->add_attempts, q->get_attempts,
         q->add_attempts - q->get_attempts);
  printf("counts (add_count: %ld; get_count: %ld; add_count - "
         "get_count: %ld)\n",
         q->add_count, q->get_count, q->add_count - q->get_count);
}
