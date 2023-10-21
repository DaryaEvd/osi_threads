#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>

#include "myThreadCreate.h"

int main(int argc, char **argv) {
  mythread_t tid1;
  void *retVal;

  printf("main: pid '%d', ppid '%d', ttid '%d'\n ", getpid(),
         getppid(), gettid());

  int resCreate1 =
      myThreadCreate(&tid1, myThreadFunc, "hello from main");
  if (resCreate1 != 0) {
    printf("error in creating thread1");
    return -1;
  }
  int resJoin1 = myThreadJoin(tid1, &retVal);
  if (resJoin1 != 0) {
    printf("error in thread joining #1");
    return -1;
  }

  mythread_t tid2;
  int resCreate2 =
      myThreadCreate(&tid2, myThreadFunc, "hello from main");
  if (resCreate2 != 0) {
    printf("error in creating thread2");
    return -1;
  }
  int resJoin2 = myThreadJoin(tid2, &retVal);
  if (resJoin2 != 0) {
    printf("error in thread joining #2");
    return -1;
  }

  printf("main: pid '%d', ppid '%d', ttid '%d' ; thread returned: "
         "'%s' \n ",
         getpid(), getppid(), gettid(), (char *)retVal);

  return 0;
}
