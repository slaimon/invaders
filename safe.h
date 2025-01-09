#ifndef __SAFE_H__
#define __SAFE_H__

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>

#define SAFE_NULL(exp, msg) \
   if ( (exp) == NULL ) { \
      perror(msg); \
      exit(EXIT_FAILURE); \
   }

#define SAFE_NEG1(exp, msg) \
   if ( (exp) == -1 ) { \
      perror(msg); \
      exit(EXIT_FAILURE); \
   }

#define SAFE_ERRNO(exp, msg) \
   if ( (errno = exp) != 0 ) { \
      perror(msg); \
      exit(EXIT_FAILURE); \
   }

// terminano il processo se l'allocazione fallisce
void* safe_malloc(size_t bytes);
void* safe_realloc(void* ptr, size_t nmemb, size_t size);

// restituisce la prima dirent leggibile, se ne trova una.
// solleva un warning per ciascuna dirent illeggibile che incontra.
// se non ne trova nessuna leggibile restituisce NULL
struct dirent* safe_readdir(DIR* dirp, char* path);

// terminano il processo se la creazione o l'inizializzazione falliscono
pthread_t safe_pthreadCreate(void* (*start_fun) (void*), void* arg);
void safe_pthreadMutexInit(pthread_mutex_t* mutex);
void safe_pthreadCondInit(pthread_cond_t* cond);

#endif
