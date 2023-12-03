#define _GNU_SOURCE

#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#include "queue-sem-impl.h"

void execMutexlock(queueT *q) {
  if (pthread_mutex_lock(&q->mutex)) {
    printf("pthread_mutex_lock() error: %s \n", strerror(errno));
    abort();
  }
}

void execMutexUnlock(queueT *q) {
  if (pthread_mutex_unlock(&q->mutex)) {
    printf("pthread_mutex_unlock() error: %s \n", strerror(errno));
    abort();
  }
}

/* wait
если 0 - ждёт
если >= 1 - не блокируется, выполняется, отнимает от текущ значения 1
*/
void execWaitSem(sem_t *sem) {
  if (sem_wait(sem) != 0) {
    printf("sem_wait() error: %s\n", strerror(errno));
  }
}

/* post - сигналит
не блокируется, выполняется, прибавляет к текущему значение 1
*/
void execPostSem(sem_t *sem) {
  if (sem_post(sem) != 0) {
    printf("sem_post() error: %s\n", strerror(errno));
  }
}

void *qmonitor(void *arg) {
  queueT *q = (queueT *)arg;

  printf("qmonitor: [%d %d %d]\n", getpid(), getppid(), gettid());

  while (1) {
    queuePrintStats(q);
    sleep(1);
  }
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
  int errSemInit = sem_init(&q->semEmpty, 0, 0); // 0 между потоками
  // 0 - начальное значение ПУСТОГО семафора (тот, что забирает
  // значения из очереди
  if (errSemInit) {
    printf("queueInit: sem_init() semEmpty failed: %s\n",
           strerror(errSemInit));
    abort();
  }

  errSemInit = sem_init(&q->semFull, 0, 1); // 0 между потоками
  // 1 - начальное значение ПОЛНОГО семафора (тот, что добавляет
  // значения в очередь
  if (errSemInit) {
    printf("queueInit: sem_init() semFull failed: %s\n",
           strerror(errSemInit));
    abort();
  }

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

void queueDestroy(queueT *q) {
  int err = pthread_cancel(q->qmonitorTid);
  if (err) {
    printf("queueDestroy(): pthread_cancel() failed: %s\n",
           strerror(err));
  }

  err = sem_destroy(&q->semEmpty);
  if (err) {
    printf("queueDestroy(): semEmpty failed: %s\n", strerror(err));
  }

  err = sem_destroy(&q->semFull);
  if (err) {
    printf("queueDestroy(): semFull failed: %s\n", strerror(err));
  }

  if (pthread_mutex_destroy(&q->mutex)) {
    printf("queueDestroy: pthread_mutex_destroy() error : %s\n",
           strerror(errno));
  }

  qnodeT *cur = q->first;
  while (cur != NULL) {
    qnodeT *next = cur->next;
    free(cur);
    cur = next;
  }

  free(q);
}

int queueAdd(queueT *q, int val) {
  /* semFull = 0: wait, block thread till > 0;
  semFull > 0: go and decrement it
  */
  execWaitSem(&q->semFull);

  execMutexlock(q);

  q->addAttempts++; // +1 попытка записать элемент

  assert(q->count <= q->maxCount);

  if (q->count == q->maxCount) {
    execMutexUnlock(q);
    execPostSem(&q->semEmpty);
    return 0;
  }

  qnodeT *new = malloc(sizeof(qnodeT)); // malloc mem for one node
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

  /*
  increment semEmpty
  */
  execPostSem(&q->semEmpty);

  return 1;
}

int queueGet(queueT *q, int *val) {
  execWaitSem(&q->semEmpty);
  execMutexlock(q);

  q->getAttempts++; // +1 попытка достать элемент
  assert(q->count >= 0);

  if (q->count == 0) {
    execMutexUnlock(q);
    execPostSem(&q->semFull);
    return 0;
  }

  qnodeT *tmp = q->first; // save ptr to the 1st node

  *val = tmp->val;           // take val of the 1st node
  q->first = q->first->next; // now next node is the 1st

  free(tmp);      // delete the 1st node
  q->count--;     // amount of elems in queue
  q->getCount++; // +1 successful попытка добавления элементов

  execMutexUnlock(q);

  /*
    increment semFull
  */
  execPostSem(&q->semFull);

  return 1;
}

void queuePrintStats(queueT *q) {
  // here we print amount of attempts и how many of them are lucky

  const int count = q->count;
  const long addAttempts = q->addAttempts;
  const long getAttempts = q->getAttempts;
  const long addCount = q->addCount;
  const long getCount = q->getCount;

  printf("queue stats: current size %d; attempts: (%ld %ld %ld); "
         "counts (%ld %ld %ld)\n",
         count, addAttempts, getAttempts,
         addAttempts - getAttempts, addCount, getCount,
         addCount - getCount);
}
