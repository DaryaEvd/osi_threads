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

  pthread_exit(NULL);
}

int main(int argc, char **argv) {
  pthread_attr_t attrs;
  if (pthread_attr_init(&attrs) != 0) {
    perror("pthread_attr_init() error");
    return -1;
  }

  if (pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED) !=
      0) {
    perror("pthread_attr_setdetachstate() error");
    return -1;
  }

  pthread_t tid;

  while (1) {
    if (pthread_create(&tid, NULL, routine, NULL) != 0) {
      perror("pthread_create() error");
      return -1;
    }
  }

  pthread_attr_destroy(&attrs);
  pthread_exit(NULL);
}
