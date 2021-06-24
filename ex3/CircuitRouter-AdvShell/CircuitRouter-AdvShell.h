#ifndef CIRCUITROUTER_SHELL_H
#define CIRCUITROUTER_SHELL_H

#include "lib/vector.h"
#include "lib/timer.h"
#include <sys/types.h>
#include <sys/time.h>


typedef struct {
    pid_t pid;
    int status;
    TIMER_T startTime;
    TIMER_T endTime;
} child_t;

void handler (int signal);
void printChildren();

#endif /* CIRCUITROUTER_SHELL_H */
