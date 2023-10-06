#define _GNU_SOURCE
#include <error.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *routine(void *arg) {
  char *mySting = "created thread: hehehe";
  return (void *)mySting;
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
    perror("pthread_create() error");
    return -1;
  }

  printf("main thread got string '%s' from created thread\n",
         (char *)retVal);

  printf("Done :) \n"); // this message is gearanteed the last
                        // cause of join()

  return 0;
}
