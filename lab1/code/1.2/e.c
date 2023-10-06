#define _GNU_SOURCE
#include <error.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void *routine(void *arg) {
  pthread_t currThread = pthread_self();
  printf("tID: %ld\n", currThread);

  if (pthread_detach(currThread) != 0) {
    perror("pthread detach() error");
  }
  return NULL;
}

int main(int argc, char **argv) {
  pthread_t tid;

  while (1) {
    if (pthread_create(&tid, NULL, routine, NULL) != 0) {
      perror("pthread_create() error");
      return -1;
    }
  }

  pthread_exit(NULL);
}
