#define _GNU_SOURCE
#include <error.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *routine(void *arg) {
  printf("Hello from created thread\n");
  return (void *)42;
}

int main(int argc, char **argv) {
  pthread_t tid;
  void *retVal;

  if (pthread_create(&tid, NULL, routine, NULL) != 0) {
    perror("pthread_create() error");
    return -1;
  }

  printf("Hello from main after creatinfg a thread\n");

  if (pthread_join(tid, &retVal) != 0) {
    perror("pthread_join() error");
    return -1;
  }

  printf("main thread got number ' %ld ' from created thread\n",
         (long)retVal);

  printf("Done :) \n"); // this message is gearanteed the last
                        // cause of join()

  return 0;
}
