#include "util.h"

#include <string.h>
#include <stdlib.h>

void die(const char * format, ...)
{
    va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, "\n");
  fflush(stderr);
  exit(1);
}

char *concat(const char *str1, const char *str2)
{
  size_t total_len = strlen(str1) + strlen(str2) + 1;
  char *newstr = malloc(sizeof(char) * total_len);
  if (newstr != NULL) {
    snprintf(newstr, total_len, "%s%s", str1, str2);
  }
  return newstr;
}

/* vim: set ts=4 sw=4 : */
