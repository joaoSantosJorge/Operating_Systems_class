/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * This code is an adaptation of the Lee algorithm's implementation originally included in the STAMP Benchmark
 * by Stanford University.
 *
 * The original copyright notice is included below.
 *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Author: Chi Cao Minh
 *
 * =============================================================================
 *
 * Unless otherwise noted, the following license applies to STAMP files:
 *
 * Copyright (c) 2007, Stanford University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of Stanford University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 *
 * CircuitRouter-SeqSolver.c
 *
 * =============================================================================
 *      make
 *      ./CircuitRouter-ParSolver ../inputs/random-x32-y32-z3-n64.txt -t 2
        ./CircuitRouter-ParSolver ../inputs/random-x512-y512-z7-n512.txt -t 2
        ./CircuitRouter-ParSolver ../inputs/random-x48-y48-z3-n64.txt -t 2


erro alocação de memória
erro destroyLocks
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "lib/list.h"
#include "maze.h"
#include "router.h"
#include "lib/timer.h"
#include "lib/types.h"
#include "mutex.h"


enum param_types {
    PARAM_BENDCOST = (unsigned char)'b',
    PARAM_XCOST    = (unsigned char)'x',
    PARAM_YCOST    = (unsigned char)'y',
    PARAM_ZCOST    = (unsigned char)'z',
};

enum param_defaults {
    PARAM_DEFAULT_BENDCOST = 1,
    PARAM_DEFAULT_XCOST    = 1,
    PARAM_DEFAULT_YCOST    = 1,
    PARAM_DEFAULT_ZCOST    = 2,
};

bool_t global_doPrint = TRUE;
char* global_inputFile = NULL;
long global_params[256]; /* 256 = ascii limit */


/* =============================================================================
 * displayUsage
 * =============================================================================
 */
static void displayUsage (const char* appName){
    printf("Usage: %s [options] input_filename\n", appName);
    puts("\nOptions:                            (defaults)\n");
    printf("    b <INT>    [b]end cost          (%i)\n", PARAM_DEFAULT_BENDCOST);
    printf("    x <UINT>   [x] movement cost    (%i)\n", PARAM_DEFAULT_XCOST);
    printf("    y <UINT>   [y] movement cost    (%i)\n", PARAM_DEFAULT_YCOST);
    printf("    z <UINT>   [z] movement cost    (%i)\n", PARAM_DEFAULT_ZCOST);
    printf("    h          [h]elp message       (false)\n");
    exit(1);
}


/* =============================================================================
 * setDefaultParams
 * =============================================================================
 */
static void setDefaultParams (){
    global_params[PARAM_BENDCOST] = PARAM_DEFAULT_BENDCOST;
    global_params[PARAM_XCOST]    = PARAM_DEFAULT_XCOST;
    global_params[PARAM_YCOST]    = PARAM_DEFAULT_YCOST;
    global_params[PARAM_ZCOST]    = PARAM_DEFAULT_ZCOST;
}


/* =============================================================================
 * parseArgs
 * =============================================================================
 */
static int parseArgs (long argc, char* const argv[]){
    long opt;
    int numTar = -1;

    opterr = 0;

    setDefaultParams();

    while ((opt = getopt(argc, argv, "hb:x:y:z:t:")) != -1) {
        switch (opt) {
            case 'b':
            case 'x':
            case 'y':
            case 'z':
                global_params[(unsigned char)opt] = atol(optarg);
                break;
            case 't':
                numTar = atoi(optarg);
                break;
            case '?':
            case 'h':
                displayUsage(argv[0]);
            default:
                break;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Missing input file\n");
        displayUsage(argv[0]);
    }

    global_inputFile = argv[optind];

    if (numTar < 1){
        fprintf(stderr,"Error numThreads\n");
        exit(EXIT_FAILURE);
    }

    return numTar;
}

/* =============================================================================
 * outputFile
 * =============================================================================
 */
FILE * outputFile() {
    FILE *fp;

    char result_outputFile[strlen(global_inputFile) + strlen(".res") + 1];
    sprintf(result_outputFile, "%s.res", global_inputFile);

    if (access(result_outputFile, F_OK) == 0) {         //se houver um ficheiro .res entra
        char old_outputFile[strlen(global_inputFile) + strlen(".res.old") + 1];
        sprintf(old_outputFile, "%s.res.old", global_inputFile);
        if (rename(result_outputFile, old_outputFile) == -1) { //muda nome do output file -> ficheiro .res passa a .res.old
            perror("Error renaming output file");
            exit(EXIT_FAILURE);;
        }
    }
    fp = fopen(result_outputFile, "wt");        //se ja houver um ficheiro .res nao interessa. reescreve sempre o ficheiro de novo
    if (fp == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }
    return fp;
}


/* =============================================================================
 * main
 * =============================================================================
 */
int main(int argc, char** argv){
    /*
     * Initialization
     */

    int numThreads = -1;
    numThreads = parseArgs(argc, argv);
    pthread_t* tid = (pthread_t*)malloc(numThreads * sizeof(pthread_t));
    FILE* resultFp = outputFile();
    maze_t* mazePtr = maze_alloc();
    assert(mazePtr);
    long numPathToRoute = maze_read(mazePtr, global_inputFile, resultFp);
    router_t* routerPtr = router_alloc(global_params[PARAM_XCOST],
                                       global_params[PARAM_YCOST],
                                       global_params[PARAM_ZCOST],
                                       global_params[PARAM_BENDCOST]);
    assert(routerPtr);
    list_t* pathVectorListPtr = list_alloc(NULL);
    assert(pathVectorListPtr);

    //long numCel = mazePtr->gridPtr->width + mazePtr->gridPtr->height + mazePtr->gridPtr->depth;
    long x = mazePtr->gridPtr->width;
    long y = mazePtr->gridPtr->height;
    long z = mazePtr->gridPtr->depth;

    mutex_t* mutexPtr = (mutex_t*)malloc(sizeof(mutex_t));
    mutexPtr->mutex_Fino = (pthread_mutex_t*)malloc(x*y*z*sizeof(pthread_mutex_t));
    
    initLock(&(mutexPtr->mutex_Queue));
    initLock(&(mutexPtr->mutex_GridCopy));
    initLock(&(mutexPtr->mutex_PathVector));

    for(int auxX = 0; auxX < x; auxX++) {
        for(int auxY = 0; auxY < y; auxY++) {
            for(int auxZ = 0; auxZ < z; auxZ++) {
                initLock(&(mutexPtr->mutex_Fino[auxX+x*(auxY+y*auxZ)]));
            }
        }
    }


    
    router_solve_arg_t routerArg = {routerPtr, mazePtr, pathVectorListPtr, mutexPtr};
    TIMER_T startTime;
    TIMER_READ(startTime);


    for (int i = 0; i < numThreads; i++) {
        if (pthread_create(&tid[i], NULL, (void *) router_solve, (void *)&routerArg) == 0) {
            printf("thread %d criada\n", i);
        }
        else {
            printf("Erro thread creation\n");
            exit(EXIT_FAILURE);
        }

    }

    for (int i = 0; i < numThreads; i++) {
        if (pthread_join(tid[i], NULL) == 0) {
            printf("Thread number %d ended\n", i);
        }
        else {
            printf("Erro join thread\n");
            exit(EXIT_FAILURE);
        }
    }

    
    //router_solve((void *)&routerArg);

    TIMER_T stopTime;
    TIMER_READ(stopTime);

    destroyLock(&(mutexPtr->mutex_Queue));
    destroyLock(&(mutexPtr->mutex_GridCopy));
    destroyLock(&(mutexPtr->mutex_PathVector));

    /*for(int auxX = 0; auxX < x; auxX++) {
        for(int auxY = 0; auxY < y; auxY++) {
            for(int auxZ = 0; auxZ < z; auxZ++) {
                destroyLock(&(mutexPtr->mutex_Fino[auxX+x*(auxY+y*auxZ)]));
            }
        }
    }*/

    long numPathRouted = 0;
    list_iter_t it;
    list_iter_reset(&it, pathVectorListPtr);
    while (list_iter_hasNext(&it, pathVectorListPtr)) {
        vector_t* pathVectorPtr = (vector_t*)list_iter_next(&it, pathVectorListPtr);
        numPathRouted += vector_getSize(pathVectorPtr);
    }
    fprintf(resultFp, "Paths routed    = %li\n", numPathRouted);
    fprintf(resultFp, "Elapsed time    = %f seconds\n", TIMER_DIFF_SECONDS(startTime, stopTime));


    /*
     * Check solution and clean up
     */
    assert(numPathRouted <= numPathToRoute);
    bool_t status = maze_checkPaths(mazePtr, pathVectorListPtr, resultFp, global_doPrint);
    assert(status == TRUE);
    fputs("Verification passed.\n",resultFp);

    
    free(tid);
    maze_free(mazePtr);
    router_free(routerPtr);

    list_iter_reset(&it, pathVectorListPtr);
    while (list_iter_hasNext(&it, pathVectorListPtr)) {
        vector_t* pathVectorPtr = (vector_t*)list_iter_next(&it, pathVectorListPtr);
        vector_t* v;
        while((v = vector_popBack(pathVectorPtr))) {
            // v stores pointers to longs stored elsewhere; no need to free them here
            vector_free(v);
        }
        vector_free(pathVectorPtr);
    }
    list_free(pathVectorListPtr);

    fclose(resultFp);
    exit(0);
}


/* =============================================================================
 *
 * End of CircuitRouter-SeqSolver.c
 *
 * =============================================================================
 */
