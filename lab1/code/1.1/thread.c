#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int globalVar = 6;

void *mythread(void *arg) {
  printf("thread [pid: %d, ppid: %d, tpid: %d]: Hello from thread!\n",
         getpid(), getppid(), gettid());
  printf("    pthread_self: %ld\n", pthread_self());

  int localVar = 7;
  static int statLocalVar = 8;
  const int constLocalVar = 1000;

  if (gettid() % 2 == 0) {
    localVar = 70;
    globalVar = 80;
  }

  printf("\t\t");

  printf("thread global\n \t val: %d \t addr: %p\n", globalVar,
         &globalVar);
  printf("\t\t thread local\n \t val: %d \t addr: %p\n", localVar,
         &localVar);
  printf("\t\t thread constLocalVar\n \t val: %d \t addr: %p\n",
         constLocalVar, &constLocalVar);
  printf("\t\t thread localStaticVar\n \t val: %d \t addr: %p\n",
         statLocalVar, &statLocalVar);

  return 0;
}

int main() {
  int amountThreads = 5;
  pthread_t tid[amountThreads];
  int err;

  printf("main [pid: %d, ppid: %d, tpid: %d]: Hello from main!\n",
         getpid(), getppid(), gettid());

  for (size_t i = 0; i < amountThreads; i++) {
    err = pthread_create(&tid[i], NULL, mythread, NULL);
    if (err) {
      printf("main: pthread_create() failed: %s\n", strerror(err));
      return -1;
    }
  }

//   sleep(15);
  pthread_exit(NULL);

//   return 0;
}
