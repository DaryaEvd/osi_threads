#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void *routine(void *args) {
  /* example for asynchronous type*/
  if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) !=
  0) {
    perror("pthread_setcanceltype() error");
    return NULL;
  }

  /* example for deffered type*/
  // это можно не писать тк по дефолту type отложенный,
  // a state - endabled
  // pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  // pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

  int counter = 0;
  while (1) {
    counter++;
    // pthread_testcancel();
  }
  return NULL;
}

int main() {
  pthread_t tid;

  if (pthread_create(&tid, NULL, routine, NULL) != 0) {
    perror("pthread_create() error");
    return -1;
  }

  if (pthread_cancel(tid) != 0) { // запрос на завершение потока
    perror("pthread_cancel() error");
    return -1;
  }

  if (pthread_join(tid, NULL) != 0) {
    perror("pthread_join() error");
    return -1;
  }

  pthread_exit(NULL);
}
