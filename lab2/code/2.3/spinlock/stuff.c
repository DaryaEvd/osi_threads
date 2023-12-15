#include "stuff.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int increasingLengthCompare(const char *str1, const char *str2) {
  return strlen(str1) > strlen(str2);
}

int decreasingLengthCompare(const char *str1, const char *str2) {
  return strlen(str1) < strlen(str2);
}

int equalLengthCompare(const char *str1, const char *str2) {
  return strlen(str1) == strlen(str2);
}