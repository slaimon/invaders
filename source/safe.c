#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "../include/safe.h"


/* ------------ OPENBSD_REALLOCARRAY ------------ */

// Had to include it for compatibility reasons.

/*
 * Copyright (c) 2008 Otto Moerbeek <otto@drijf.net>
 * SPDX-License-Identifier: MIT
 */

#ifndef SIZE_MAX
    #define SIZE_MAX     UINTPTR_MAX
#endif

/*
 * This is sqrt(SIZE_MAX+1), as s1*s2 <= SIZE_MAX
 * if both s1 < MUL_NO_OVERFLOW and s2 < MUL_NO_OVERFLOW
 */
#define MUL_NO_OVERFLOW	((size_t)1 << (sizeof(size_t) * 4))

void * openbsd_reallocarray(void *optr, size_t nmemb, size_t size) {
	if ((nmemb >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) &&
	    nmemb > 0 && SIZE_MAX / nmemb < size) {
		errno = ENOMEM;
		return NULL;
	}
	if (size == 0 || nmemb == 0)
	    return NULL;
	return realloc(optr, size * nmemb);
}
/* ---------------------------------------------- */

FILE* safe_fopen(const char* fname, const char* mode) {
	FILE* ptr = fopen(fname, mode);
	
	if (ptr == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	
	return ptr;
}

void* safe_malloc(size_t bytes) {
	void* ptr = malloc(bytes);
	
	if (ptr == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	
	return ptr;
}

void* safe_realloc(void* ptr, size_t nmemb, size_t size) {
   void* newptr = openbsd_reallocarray(ptr, nmemb, size);
   
   if (newptr == NULL) {
      perror("reallocarray");
      exit(EXIT_FAILURE);
   }
   
   return newptr;
}
