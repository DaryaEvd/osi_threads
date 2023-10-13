#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

static void cleanupHandler(void *arg) {
  free((char *)arg);
  printf("clean handler finished\n");
}

void *routine(void *args) {
  const int strLength = 12;
  char *str = malloc(sizeof(char) * strLength);
  if (!str) {
    perror("malloc() error");
    return NULL;
  }

  str = strncpy(str, "hello World", strLength);

  pthread_cleanup_push(cleanupHandler, str);

  while (1) {
    printf("%s\n", str);
    pthread_testcancel();
  }

  pthread_cleanup_pop(1);
  return NULL;
}

int main() {
  pthread_t tid;

  if (pthread_create(&tid, NULL, routine, NULL) != 0) {
    perror("pthread_create() error");
    return -1;
  }

  if (pthread_cancel(tid) != 0) { // запрос на завершение потока
    perror("pthread_cancel() error");
    return -1;
  }

  if (pthread_join(tid, NULL) != 0) {
    perror("pthread_join() error");
    return -1;
  }

  pthread_exit(NULL);
}
