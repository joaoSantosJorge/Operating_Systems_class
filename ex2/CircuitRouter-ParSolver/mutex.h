#ifndef MUTEX_H
#define MUTEX_H 1

#include <pthread.h>
#include <stdbool.h>

typedef struct mutex {
	pthread_mutex_t mutex_Queue;
	pthread_mutex_t mutex_GridCopy;
	pthread_mutex_t mutex_PathVector;
	pthread_mutex_t* mutex_Fino;
} mutex_t;



void initLock (pthread_mutex_t* mutex);
void destroyLock (pthread_mutex_t* mutex);
void safeLock (pthread_mutex_t* mutex);
void safeUnlock (pthread_mutex_t* mutex);
bool safeTrylock (pthread_mutex_t* mutex);


#endif