#include "stuff.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

char *generateRandomString(char *randomStr) {
  static char *charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOP"
                         "QRSTUVWXYZ0123456789";
  size_t randomLengthStr =
      (1.0 + 100.0 * rand() / RAND_MAX); // nonzero

  for (size_t i = 0; i < randomLengthStr - 1; i++) {
    size_t randSymb = rand() % (sizeof(charset) - 1) % 5;
    randomStr[i] = charset[randSymb];
  }
  randomStr[randomLengthStr] = '\0';

  return randomStr;
}
