#define _GNU_SOURCE
#include <error.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *routine(void *arg) {
  printf("Hello from created thread\n");
  return NULL;
}

int main(int argc, char **argv) {
  pthread_t tid;
  void *ret;

  if (pthread_create(&tid, NULL, routine, NULL) != 0) {
    perror("pthread_create() error");
    return -1;
  }

  printf("Hello from main after creatinfg a thread\n");

  if (pthread_join(tid, &ret) != 0) {
    perror("pthread_create() error");
    return -1;
  }

  printf("Done :) \n"); // this message is gearanteed the last cause
                        // of join()

  return 0;
}
