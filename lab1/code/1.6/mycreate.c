#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PAGE 4096
#define STACK_SIZE 3 * PAGE
#define STACK_FILE_SIZE 128

// a ptr to a function that recieves void * and returns void*
typedef void *(*start_routine_t)(void *);

typedef struct mythread {
  int mythreadID;
  void *arg;
  start_routine_t startRoutine;
  void *retVal;
  volatile int finished;
  volatile int joined;
} myThreadStruct;

/* a ptr to struct myThreadStruct */
typedef myThreadStruct *mythread_t;

void *createStack(off_t size, int mytid) {
  char stackFile[STACK_FILE_SIZE];

  // returns ampount of wtritten symbols
  if (snprintf(stackFile, sizeof(stackFile), "stack-%d", mytid) < 0) {
    perror("snprintf() error");
    return NULL;
  }

  int stackFileDescriptor = open(stackFile, O_RDWR | O_CREAT, 0660);
  if (stackFileDescriptor < 0) {
    perror("open() error");
    return NULL;
  }

  // обрезаем файл
  if (ftruncate(stackFileDescriptor, 0) == -1) {
    perror("ftruncate() error");
    return NULL;
  }
  if (ftruncate(stackFileDescriptor, size) == -1) {
    perror("ftruncate() error");
    return NULL;
  }

  void *stack =
      mmap(NULL, size, PROT_NONE, MAP_SHARED, stackFileDescriptor, 0);
  if (stack == NULL) {
    perror("mmap() error");
    return NULL;
  }

  if (close(stackFileDescriptor) == -1) {
    perror("close() error");
    return NULL;
  }

  printf("createStack() : created for thread#%d\n", mytid);

  return stack;
}

// wrapper function
/* to have more additional control for thread function,
we can do smth before and after calling thread func
*/
int threadStart(void *arg) {
  mythread_t tid = (mythread_t)arg;
  myThreadStruct *thread = tid;
  printf(
      "thread start: starting a thread functon for thread num %d\n",
      thread->mythreadID);

  void *retVal = thread->startRoutine(thread->arg);
  thread->retVal = retVal;
  thread->finished = 1;

  printf("thread start: waiting for join() thread num %d\n",
         thread->mythreadID);

  while (!thread->joined) {
    sleep(1);
  }

  printf("thread start: thread function finished for thread num %d\n",
         thread->mythreadID);

  return 0;
}

int myThreadCreate(mythread_t *mytid, void *(*startRoutine)(void *),
                   void *arg) {
  static int mythreadID = 0;

  mythreadID++;

  printf("myThreadCreate(): creating thread num: %d\n", mythreadID);

  void *childStack = createStack(STACK_SIZE, mythreadID);
  if (childStack == NULL) {
    return -1;
  }

  if (mprotect(childStack + PAGE, STACK_SIZE - PAGE,
               PROT_READ | PROT_WRITE) == -1) {
    printf("mprotect() error");
    return -1;
  }

  if (memset(childStack + PAGE, 0x7f, STACK_SIZE - PAGE) == NULL) {
    printf("memset() error");
    return -1;
  }

  myThreadStruct *thread =
      (myThreadStruct *)(childStack + STACK_SIZE -
                         sizeof(myThreadStruct));

  thread->mythreadID = mythreadID;
  thread->arg = arg;
  thread->startRoutine = startRoutine;
  thread->retVal = NULL;
  thread->finished = 0;
  thread->joined = 0;

  childStack = (void *)thread;

  printf("child stack: %p; mythread_struct %p: \n", childStack,
         thread);

  int childPid = clone(
      threadStart, childStack,
      CLONE_VM |        // the calling process and the child process
                        // run in the same memory space
          CLONE_FILES | // the calling process and the child process
                        // share the same file descriptor table.

          CLONE_THREAD |  // the child is placed in the same thread
                          // group  as the  calling process.
          CLONE_SIGHAND | // the calling process and the child process
                          // share  the same table of signal handlers.
          SIGCHLD,        /*    After all of the threads in a
                      thread group terminate the  parent  process
             of the thread group is sent a SIGCHLD(or other
             termination) signal */
      thread);

  if (childPid == -1) {
    printf("clone() error: %s\n", strerror(errno));
    exit(-1);
  }
  *mytid = thread;

  return 0;
}

int mythread_join(mythread_t mytid, void **retVal) {
  myThreadStruct *thread = mytid;
  printf("thread_join: waiting for thread num '%d' to finish\n",
         thread->mythreadID);
  while (!thread->finished) {
    usleep(1);
  }

  printf("thread_join: thread num '%d' finished\n",
         thread->mythreadID);

  *retVal = thread->retVal;
  thread->joined = 1;

  return 0;
}

void *mythread(void *arg) {
  char *str = (char *)arg;
  for (int i = 0; i < 5; i++) {
    printf("hello from my thread '%s'\n", str);
    sleep(1);
  }
  return "bye";
}

int main(int argc, char **argv) {
  mythread_t tid1;
  void *retVal;

  printf("main: pid '%d', ppid '%d', ttid '%d'\n ", getpid(),
         getppid(), gettid());

  int resCreate1 = myThreadCreate(&tid1, mythread, "hello from main");
  if (resCreate1 != 0) {
    printf("error in creating thread1");
    return -1;
  }
  int resJoin1 = mythread_join(tid1, &retVal);
  if (resJoin1 != 0) {
    printf("error in thread joining #1");
    return -1;
  }

  mythread_t tid2;
  int resCreate2 = myThreadCreate(&tid2, mythread, "hello from main");
  if (resCreate2 != 0) {
    printf("error in creating thread2");
    return -1;
  }
  int resJoin2 = mythread_join(tid2, &retVal);
  if (resJoin2 != 0) {
    printf("error in thread joining #2");
    return -1;
  }

  printf("main: pid '%d', ppid '%d', ttid '%d' ; thread returned: "
         "'%s' \n ",
         getpid(), getppid(), gettid(), (char *)retVal);

  return 0;
}
