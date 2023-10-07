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
  printf("in created thread: name is '%s'; number is '%d'\n",
         ((PhoneBook *)arg)->name, ((PhoneBook *)arg)->number);
  return NULL;
}

int main(int argc, char **argv) {
  PhoneBook *phonebook = malloc(sizeof(PhoneBook));
  if (!phonebook) {
    perror("malloc() error");
    return -1;
  }

  phonebook->name = "dasha";
  phonebook->number = 134567;

  printf("when malloc: name: '%s' and number '%d'\n", phonebook->name,
         phonebook->number);

  pthread_t tid;
  pthread_attr_t attrs;
  if (pthread_attr_init(&attrs) != 0) {
    perror("error in pthread_attr_init");
    return -1;
  }

  if (pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED) !=
      0) {
    perror("pthread_attr_setdetachstate error()");
    return -1;
  }

  if (pthread_create(&tid, NULL, routine, &phonebook) != 0) {
    perror("pthread create() error");
    return -1;
  }

  free(phonebook);
  pthread_attr_destroy(&attrs);
  pthread_exit(NULL);
}
