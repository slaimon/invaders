#ifndef __SAFE_H__
#define __SAFE_H__

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>

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
void* safe_realloc(void* ptr, size_t nmemb, size_t size);

FILE* safe_fopen(const char* fname, const char* mode);

#endif
