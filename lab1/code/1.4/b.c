#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void *routine(void *args) {
  // if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0) {
  //   perror("pthread_setcanceltype() error");
  //   return NULL;
  // }
/*с deffered сделать !!!!!!!!*/
  int counter = 0;
  while (1) {
    counter++;
  }
}

int main() {
  pthread_t tid;

  if (pthread_create(&tid, NULL, routine, NULL) != 0) {
    perror("pthread_create() error");
    return -1;
  }

  if (pthread_cancel(tid) != 0) {
    perror("pthread_cancel() error");
    return -1;
  }

  pthread_exit(NULL);
}
