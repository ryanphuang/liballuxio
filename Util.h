/**
 *  @author        Ryan Huang <ryanhuang@cs.ucsd.edu>
 *  @organization  University of California, San Diego
 * 
 * Handy util functions
 *
 */

#ifndef __UTIL_H_
#define __UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

// print error message
#define warn(format, ...) do { \
    fprintf(stderr, format,  __VA_ARGS__); \
} while (0)

// print error message and exit
void die(const char *, ...) __attribute__ ((noreturn, format (printf, 1, 2)));

// concatenate two strings
char *concat(const char *, const char *);

// find the character next to a given character
char findNext(const char *, char);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UTIL_H_ */

/* vim: set ts=4 sw=4 : */
