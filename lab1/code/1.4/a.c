#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void *routine(void *args) {
  while (1) {
    printf("This is mythread!\n");
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
  
  if (pthread_join(tid, NULL) != 0) {
    perror("pthread_join() error");
    return -1;
  }

  pthread_exit(NULL);
}
