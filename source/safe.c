#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <dirent.h>

#include "../include/safe.h"

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
   void* newptr = reallocarray(ptr, nmemb, size);
   
   if (newptr == NULL) {
      perror("reallocarray");
      exit(EXIT_FAILURE);
   }
   
   return newptr;
}
