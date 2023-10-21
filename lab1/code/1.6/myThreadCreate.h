#ifndef MY_THREAD_H
#define MY_THREAD_H

#include <unistd.h>

// a ptr to a function that recieves void * and returns void*
typedef void *(*startRoutine)(void *);

typedef struct mythread {
  int mythreadID;
  void *arg;
  startRoutine startRoutine;
  void *retVal;
  volatile int finished;
  volatile int joined;
} myThreadStruct;

/* a ptr to struct myThreadStruct */
typedef myThreadStruct *mythread_t;

void *createStack(off_t size, int mytid);
int myThreadStart(void *arg);
int myThreadCreate(mythread_t *mytid, void *(*startRoutine)(void *),
                   void *arg);
int myThreadJoin(mythread_t mytid, void **retVal);
void *myThreadFunc(void *arg);

#endif // MY_THREAD_H
