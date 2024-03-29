#define _GNU_SOURCE
#include <error.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct MyStruct {
  int number;
  char *name;
} PhoneBook;

void *routine(void *arg) {
  printf("name is '%s'; number is '%d'\n", ((PhoneBook *)arg)->name,
         ((PhoneBook *)arg)->number);
  return NULL ;
}

int main(int argc, char **argv) {
  PhoneBook phonebook;
  phonebook.name = "dasha";
  phonebook.number = 134567;

  pthread_t tid;
  if (pthread_create(&tid, NULL, routine, &phonebook) != 0) {
    perror("pthread create() error");
    return -1;
  }

  void *retVal;
  if (pthread_join(tid, &retVal) != 0) {
    perror("pthread join() error");
    return -1;
  }

  pthread_exit(NULL);
}
