#define _GNU_SOURCE

#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#include "queue-sem-impl.h"

void execWaitSem(queue_t *q) {
  if (sem_wait(&q->sem) != 0) {
    printf("sem_wait() error: %s\n", strerror(errno));
  }
}

void execPostSem(queue_t *q) {
  if (sem_post(&q->sem) != 0) {
    printf("sem_post() error: %s\n", strerror(errno));
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

  /*
  sem_init()  initializes  the unnamed semaphore at the address
  pointed to by sem. The value argument specifies the initial value
  for the semaphore.

       The pshared argument indicates whether this semaphore is to  be
  shared  between the threads of a process, or between processes.

       If  pshared has the value 0, then the semaphore is shared
  between the threads of a process, and should be located at some
  address that is visible to all  threads (e.g., a global variable, or
  a variable allocated dynamically on the heap).

       If  pshared  is  nonzero,  then  the  semaphore is shared
  between processes, and should be located in a region of shared
  memory (see  shm_open(3),  mmap(2),  and shmget(2)).  (Since a child
  created by fork(2) inherits its parent's memory map‐ pings, it can
  also access the semaphore.)   Any  process  that  can  access  the
       shared   memory   region   can  operate  on  the  semaphore
  using  sem_post(3), sem_wait(3), and so on.
  */
  int errSemInit = sem_init(&q->sem, 0, 1);
  if (errSemInit) {
    printf("queue_init: sem_init() failed: %s\n",
           strerror(errSemInit));
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

  err = sem_destroy(&q->sem);
  if (err) {
    printf("queue_destroy(): () failed: %s\n", strerror(err));
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
  execWaitSem(q);
  q->add_attempts++; // +1 попытка записать элемент

  assert(q->count <= q->max_count);

  if (q->count == q->max_count) {
    execPostSem(q);
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

  execPostSem(q);

  return 1;
}

int queue_get(queue_t *q, int *val) {
  execWaitSem(q);

  q->get_attempts++; // +1 попытка достать элемент
  assert(q->count >= 0);

  if (q->count == 0) {
    execPostSem(q);
    return 0;
  }

  qnode_t *tmp = q->first; // save ptr to the 1st node

  *val = tmp->val;           // take val of the 1st node
  q->first = q->first->next; // now next node is the 1st

  free(tmp);      // delete the 1st node
  q->count--;     // amount of elems in queue
  q->get_count++; // +1 successful попытка добавления элементов

  execPostSem(q);

  return 1;
}

void queue_print_stats(queue_t *q) {
  // here we print amount of attempts и how many of them are lucky

  execWaitSem(q);

  const int count = q->count;
  const long add_attempts = q->add_attempts;
  const long get_attempts = q->get_attempts;
  const long add_count = q->add_count;
  const long get_count = q->get_count;

  execPostSem(q);

  printf("queue stats: current size %d; attempts: (%ld %ld %ld); "
         "counts (%ld %ld %ld)\n",
         count, add_attempts, get_attempts,
         add_attempts - get_attempts, add_count, get_count,
         add_count - get_count);
}
