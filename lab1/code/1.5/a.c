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

#define AMOUNT_TIMES 5

void allProcsHandler(int signum) {
  const char *msg = "Recieved SIGINT signal, routine#1 \n";
  write(1, msg, strlen(msg));
}

void *blockSignalsRouting(void *args) {
  sigset_t
      set; // A set of signals to be blocked, unblocked, or waited
  
   if (signal(SIGINT, allProcsHandler) == SIG_ERR) {
    perror("signal() error");
    // pthread_exit(1);
    return NULL;
  }

  if (sigfillset(&set) == -1) {
    perror("sigfillset() error in 1st routine");
    return NULL;
  }
  if (pthread_sigmask(SIG_SETMASK, &set, NULL) != 0) {
    perror("pthread_sigmask() error in 1st routine");
    return NULL;
  }

  int counter1 = 0;
  // for (size_t i = 0; i < AMOUNT_TIMES; i++) {
  while (1) {
    sleep(1);
    counter1++;
    printf("counter#1: %d\n", counter1);
  }
  return NULL;
}

void sigIntHandler(int signum) {
  const char *msg = "Received SIGINT signal, routine#2 \n";
  write(1, msg, strlen(msg));
}

void *handleSigIntRouting(void *args) {
  if (signal(SIGINT, sigIntHandler) == SIG_ERR) {
    perror("signal() error");
    // pthread_exit(1);
    return NULL;
  }

  int counter2 = 0;
  // for (size_t i = 0; i < AMOUNT_TIMES; i++) {
  while (1) {
    sleep(1);
    counter2++;
    printf("counter#2: %d\n", counter2);
  }
  return NULL;
}

void *sigQuitRouting(void *args) {
  int storeRecvSignals;

  sigset_t set;

  if (sigaddset(&set, SIGQUIT) == -1) {
    perror("sigaddset() error in #3");
    return NULL;
  }

  if (sigwait(&set, &storeRecvSignals) != 0) {
    perror("sigwait() error()\n");
    return NULL;
  }

  printf("received SIGQUIT signal, #3\n");

  int counter3 = 0;
  while (1) {
    sleep(1);
    counter3++;
    printf("counter#3: %d\n", counter3);
  }

  // pthread_exit(NULL);
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

  sleep(3);
  if (pthread_kill(thread1, SIGINT)) {
    perror("pthread_kill error in 1st");
    return -1;
  }

  if (pthread_kill(thread2, SIGINT)) {
    perror("pthread_kill error in 2nd");
    return -1;
  }

  if (pthread_kill(thread3, SIGQUIT)) {
    perror("pthread_kill error in 3rd");
    return -1;
  }

  while (1) {
    printf("\t\t main: hello to all tids\n");
    sleep(1);
  }

  return 0;
}
