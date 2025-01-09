#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <dirent.h>

#include "safe.h"

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

struct dirent* safe_readdir(DIR* dirp, char* path) {
   errno = 0;
   int error;
   struct dirent* entry = readdir(dirp);
   
   while (entry == NULL && errno != 0) {
      error = errno;
      fprintf(stderr, "skipping an entry because of error while reading directory %s: ", path);
      errno = error;
      perror("");
      entry = readdir(dirp);
   }
   
   return entry;
}

pthread_t safe_pthreadCreate(void* (*start_fun) (void*), void* arg) {
   pthread_t tid;
   int retval = pthread_create(&tid, NULL, start_fun, arg);
   
   if(retval != 0) {
      errno = retval;
      perror("pthread_create");
      exit(EXIT_FAILURE);
   }
   
   return tid;
}

void safe_pthreadMutexInit(pthread_mutex_t* mutex) {
   if (pthread_mutex_init(mutex, NULL) != 0) {
	   perror("pthread_mutex_init");
	   exit(EXIT_FAILURE);
	}
}

void safe_pthreadCondInit(pthread_cond_t* cond) {
   if (pthread_cond_init(cond, NULL) != 0) {
      perror("pthread_cond_init");
      exit(EXIT_FAILURE);
   }
}
