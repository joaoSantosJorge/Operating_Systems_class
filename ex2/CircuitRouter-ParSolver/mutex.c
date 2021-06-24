#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "mutex.h"

void initLock (pthread_mutex_t* mutex) {
	if (pthread_mutex_init(mutex, NULL) != 0) {
		fprintf(stderr, "Erro initLock\n");
		exit(EXIT_FAILURE);
	}
}

void destroyLock (pthread_mutex_t* mutex) {
	if (pthread_mutex_destroy(mutex) != 0) {
		fprintf(stderr, "Erro destroyLock\n");
		exit(EXIT_FAILURE);
	}
}

void safeLock (pthread_mutex_t* mutex) {
	if (pthread_mutex_lock(mutex) != 0) {
		fprintf(stderr, "Erro safeLock\n");
		exit(EXIT_FAILURE);
	}
}

void safeUnlock (pthread_mutex_t* mutex) {
	if (pthread_mutex_unlock(mutex) != 0) {
		fprintf(stderr, "Erro safeUnLock\n");
		exit(EXIT_FAILURE);
	}
}

bool safeTrylock (pthread_mutex_t* mutex) {
	if (pthread_mutex_trylock(mutex) == 0) {
		return true;
	}
	else {
		/*if (errno != EBUSY) {
			fprintf(stderr, "Erro safeTrylock\n");
			exit(EXIT_FAILURE);
		}
		else {
			return false;
		}*/
		return false;
	}
}
