#ifndef __UTIL_H_
#define __UTIL_H_

#define warn(format, ...) do { \
    fprintf(stderr, format,  __VA_ARGS__); \
} while (0)

void die(const char *, ...) __attribute__ ((noreturn, format (printf, 1, 2)));

char *concat(const char *, const char *)

#endif /* __UTIL_H_ */

/* vim: set ts=4 sw=4 : */
