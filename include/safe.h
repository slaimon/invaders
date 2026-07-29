#ifndef __SAFE_H__
#define __SAFE_H__

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>

#define SAFE_NULL(exp, msg)   \
   if ( (exp) == NULL ) {     \
      perror(msg);            \
      exit(EXIT_FAILURE);     \
   }

#define SAFE_NEG1(exp, msg)   \
   if ( (exp) == -1 ) {       \
      perror(msg);            \
      exit(EXIT_FAILURE);     \
   }

#define SAFE_ERRNO(exp, msg)     \
   if ( (errno = exp) != 0 ) {   \
      perror(msg);               \
      exit(EXIT_FAILURE);        \
   }

void* safe_malloc(size_t bytes);

FILE* safe_fopen(const char* fname, const char* mode);

// Buffer-limited strlen. Will return L + 1, where L is either maxlen or the
// number of consecutive nonzero bytes starting at the location pointed to by
// str, whichever is smaller.
size_t safe_strlen(const char* str, size_t maxlen);

#endif
