#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define BLOCK_ALL_SIGNALS 1
#define USES_SIGNAL_HANDLER 2
#define USES_SIGWAIT 3

void *blockSignalsRouting(void *args) {
  sigset_t
      set; // A set of signals to be blocked, unblocked, or waited for

  pthread_sigmask(SIG_BLOCK, &set, NULL);

  int counter1 = 0;
  while (1) {
    counter1++;
  }
}

void sigIntHandler(int signum) { printf("Recieved SIGINT signal\n"); }

void *handleSigIntRouting(void *args) {
  if (signal(SIGINT, sigIntHandler) == SIG_ERR) {
    perror("signal() error");
    // pthread_exit(1);
    return NULL;
  }

  int counter2 = 0;
  while (1) {
    counter2++;
  }
}

void *sigQuitRouting(void *args) {
  int storeRecvSignals;

  sigset_t set;
  sigwait(&set, &storeRecvSignals);

  printf("reveived SIGQUIT signal\n");

  pthread_exit(NULL);
}

int main(int argc, char **argv) {

  pthread_t thread1, thread2, thread3;

  if (pthread_create(&thread1, NULL, blockSignalsRouting, NULL) !=
      0) {
    perror("pthread_create() error in thread #1");
    return -1;
  }

  if (pthread_create(&thread2, NULL, handleSigIntRouting, NULL) !=
      0) {
    perror("pthread_create() error in thread #2");
    return -1;
  }

  if (pthread_create(&thread3, NULL, sigQuitRouting, NULL) != 0) {
    perror("pthread_create() error in thread #3");
    return -1;
  }

  if (pthread_join(thread1, NULL) != 0) {
    perror("pthread_join() error in thread #1");
    return -1;
  }

  if (pthread_join(thread2, NULL) != 0) {
    perror("pthread_join() error in thread #2");
    return -1;
  }

  if (pthread_join(thread3, NULL) != 0) {
    perror("pthread_join() error in thread #3");
    return -1;
  }

  return 0;
}
