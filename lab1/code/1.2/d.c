#define _GNU_SOURCE
#include <error.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void *routine(void *arg) {
  printf("thread [pid: %d, ppid: %d, tpid: %d]: Hello from thread!\n",
         getpid(), getppid(), gettid());
  pthread_exit(NULL);
}

int main(int argc, char **argv) {

  while (1) {
    pthread_t tid;
    if (pthread_create(&tid, NULL, routine, NULL) != 0) {
      perror("pthread_create() error");
      return -1;
    }
  }

  pthread_exit(NULL);
}
