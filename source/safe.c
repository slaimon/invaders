#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>

#include "../include/safe.h"

FILE* safe_fopen(const char* fname, const char* mode) {
	FILE* ptr = fopen(fname, mode);
	char msg[] = "failed to open file %s";
	char* fmt = safe_malloc(sizeof(msg) + strlen(fname));
	
	if (ptr == NULL) {
		sprintf(fmt, msg, fname);
		perror(fmt);
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
