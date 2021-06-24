
/*
// Projeto SO - exercise 1, version 1
// Sistemas Operativos, DEI/IST/ULisboa 2018-19

make 
    ./CircuitRouter-SimpleShell 10
    run ../inputs/random-x32-y32-z3-n64.txt


pipe bidireccionais
um pipe por cliente

identificar pipe com pid

mktemp (nao deve ser utilizado) -----> mkstemp


2a parte
criar thread

ou

chamada select ( fd -> fileDescriptor + 1, &setfd, NULL, NULL, NULL) --> [verificar erro -1 e errno == EINTR] -> se isto acontecer voltar a chamar select
					obter fd do pipe fileno, fileno -> retorna fd a partir de FILE*
					fd = fd do pipe +1

					setfd FD_SET -> declarar variavel deste tipo
					FD_ZERO
					FD_SET (x2)


					fazer FD_SET antes do select
					select
					if (F_ISSET(stdin))
					...
					if(F_ISSET(pipe))
					...

-------------------------------------------------
mktemp
mkfifo (nomeDoPipe, 0777, ) ----> se der erro criar directoria /tmp/
fopen fclose	fprintf -> depois usar flush para forcar a escrita
					-> n-- r = O_RDONLY|O_NONBLOCK (leitura, bloqueante)
					-> r+ = O_RDWt (leitura , nao bloqueante)
					-> a = O_WRONLY|O_CREATE|O_APPEND		(access()-> verificar se shell existe)

ler do pipe no client -> fgets
na shell utilizar readlinearguments (a unica coisa a fazer e adicionar o identificador do pipe no readLineArg)
					BUFFER_SIZE tem de ser igual na shell e no client




Signals
 no inicio do codigo utiliza se signal (SIGPIPE, SIg_ ) -> fflish = EPIPE
nao usar chamada signal
usar sigaction


O_RDONLY|O_NONBLOCK
depois select

7 pipe/fifo
2 open(ler flags)
2 select

7 signal
7 signal-safety

utilizar 2 sigaction em vez de signal


3 mkdtemp
depois criar fifo


signals (UTILIZAR SIGACTION)
->alterar estrutura referente filhos (child_t), meter tempo inicial, tempo fim execucao
ler tempo inicial comeca filho
-> marcar tempo antes fork

variavel tipo 
    struct sigaction xpto;
    xpto.sa_handler = rotinaTratamento; --> nome funcao
    xpto.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&xpto.sa_mask);
    sigaction(SIGCHILD, &xpto, NULL);


-->sigprocmask

quanto terminar filho (SIGCHILD) ir para rotina tratamento


Se entro no SIGCHILD assumor que terminam mais do que um proc filho ou seja
waitpid(-1, status, WNOHANG) dentro de um while

exit nao faz sentido saber tempo
usar  sigprocmask --> antes fork, exit quando se atinge MAXCHILDREN
-----------------------------------------------------------------------------
SIGPIPE: bloquear 
no write e/fflush se -1 e errno = EPIPE


sigmask
sigblock
*/

#include "lib/commandlinereader.h"
#include "lib/vector.h"
#include "CircuitRouter-AdvShell.h"
#include "lib/timer.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <signal.h>

#define COMMAND_EXIT "exit"
#define COMMAND_RUN "run"
#define PERMISSIONS 0777

#define MAXARGS 3
#define BUFFER_SIZE 100

int runningChildren = 0;
vector_t *children;

void handler (int signal) {
    for(int i = 0; i < vector_getSize(children); i++) {
        child_t* child = vector_at(children, i);
        if (waitpid(child->pid, &(child->status), WNOHANG) == -1) {
            if (errno = ECHILD)
                continue;
            perror("Error waitpid");
            exit(EXIT_FAILURE);
        }
        if (child->pid < 0) {
            if (errno == EINTR) {
                /* Este codigo de erro significa que chegou signal que interrompeu a espera
                   pela terminacao de filho; logo voltamos a esperar */
                free(child);
            } else {
                perror("Unexpected error while waiting for child.");
                exit (EXIT_FAILURE);
            }
        }
        TIMER_READ(child->endTime);
        runningChildren--;
    }
}

void printChildren() {
    for (int i = 0; i < vector_getSize(children); ++i) {
        child_t *child = vector_at(children, i);
        int status = child->status;
        pid_t pid = child->pid;

        if (pid != -1) {
            const char* ret = "NOK";
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                ret = "OK";
            }
            printf("CHILD EXITED: (PID=%d; return %s; %f s)\n", pid, ret, TIMER_DIFF_SECONDS(child->startTime, child->endTime));
        }
    }
    puts("END.");
}

int main (int argc, char** argv) {

    char *args[MAXARGS + 1];
    char buffer[BUFFER_SIZE];
    int MAXCHILDREN = -1;
    FILE* fd;
    fd_set setfd;
    struct sigaction check;
    check.sa_handler = handler;
    check.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    if (sigemptyset(&check.sa_mask) != 0) {
        perror("Error sigemptyset");
        exit(EXIT_FAILURE);
    }

    sigaction(SIGCHLD, &check, NULL);

    if(argv[1] != NULL){
        MAXCHILDREN = atoi(argv[1]);
    }

    children = vector_alloc(MAXCHILDREN);

    char* pathName = argv[0];
    strcat(pathName, ".pipe");

    int pipeExists = access(pathName, F_OK);
    if (pipeExists == 0) {
        if (unlink(pathName) != 0) {
            perror("Error unlink");
            exit(EXIT_FAILURE);
        }
    }

    if (mkfifo(pathName, PERMISSIONS) != 0) {
        perror("Error mkfifo");
        exit(EXIT_FAILURE);
    }

    fd = fopen(pathName, "r+");
    if (fd == NULL) {
        perror("Error fopen");
        exit(EXIT_FAILURE);
    }

    int fileDescriptor = fileno(fd);
    if (fileDescriptor == -1) {
        perror("Error fileno");
        exit(EXIT_FAILURE);
    }


    printf("Welcome to CircuitRouter-SimpleShell\n\n");

    while (1) {

        int numArgs = 0;

        FD_ZERO(&setfd);
        FD_SET(STDIN_FILENO, &setfd);
        FD_SET(fileDescriptor, &setfd);

        while (select(fileDescriptor + 1, &setfd, NULL, NULL, NULL) == -1) {
            if (errno == EINTR)
                continue;
            perror("Error select");
            exit(EXIT_FAILURE);
        }


        if (FD_ISSET(fileDescriptor,&setfd)) {
            numArgs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE, fd);

                /* EOF (end of file) do stdin ou comando "sair" */
            if (numArgs < 0 || (numArgs > 0 && (strcmp(args[0], COMMAND_RUN) != 0))) {
                char* pipeName = args[2];
                FILE* returnPipe;

                returnPipe = fopen(pipeName, "a");
                if (returnPipe == NULL) {
                    perror("Error fopen");
                    exit(EXIT_FAILURE);
                }

                fprintf(returnPipe, "Command not supported\n");
                fflush(returnPipe);


                if (fclose(returnPipe) != 0) {
                    perror("Error fclose");
                    exit(EXIT_FAILURE);
                }
            }

            else if (numArgs > 0 && strcmp(args[0], COMMAND_RUN) == 0){
                int pid;
                if (numArgs < 2) {
                    printf("%s: invalid syntax. Try again.\n", COMMAND_RUN);
                    continue;
                }
                if (MAXCHILDREN != -1 && runningChildren >= MAXCHILDREN) {
                    pause();
                }

                child_t *child = malloc(sizeof(child_t));
                if (child == NULL) {
                    perror("Error allocating memory");
                    exit(EXIT_FAILURE);
                }
                TIMER_READ(child->startTime);

                pid = fork();
                if (pid < 0) {
                    perror("Failed to create new process.");
                    exit(EXIT_FAILURE);
                }

                if (pid > 0) {
                    child->pid = pid;
                    vector_pushBack(children, child);
                    runningChildren++;
                    printf("%s: background child started with PID %d.\n\n", COMMAND_RUN, pid);
                    continue;
                } else {
                    char seqsolver[] = "../CircuitRouter-SeqSolver/CircuitRouter-SeqSolver";
                    char *newArgs[4] = {seqsolver, args[1], args[2], NULL};

                    execv(seqsolver, newArgs);
                    perror("Error while executing child process"); // Nao deveria chegar aqui
                    exit(EXIT_FAILURE);
                }
            }

            else if (numArgs == 0){
                /* Nenhum argumento; ignora e volta a pedir */
                continue;
            }
            else
                printf("Unknown command. Try again.\n");

        }

        if (FD_ISSET(STDIN_FILENO,&setfd)) {
            numArgs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE, stdin);


                /* EOF (end of file) do stdin ou comando "sair" */
            if (numArgs < 0 || (numArgs > 0 && (strcmp(args[0], COMMAND_EXIT) == 0))) {
                printf("CircuitRouter-SimpleShell will exit.\n--\n");

                /* Espera pela terminacao de cada filho */
                while (runningChildren > 0) {
                    pause();
                }

                printChildren();
                printf("--\nCircuitRouter-SimpleShell ended.\n");
                break;
            }

            else if (numArgs > 0 && strcmp(args[0], COMMAND_RUN) == 0){
                int pid;
                if (numArgs < 2) {
                    printf("%s: invalid syntax. Try again.\n", COMMAND_RUN);
                    continue;
                }
                if (MAXCHILDREN != -1 && runningChildren >= MAXCHILDREN) {
                    pause();
                }

                child_t *child = malloc(sizeof(child_t));
                if (child == NULL) {
                    perror("Error allocating memory");
                    exit(EXIT_FAILURE);
                }
                TIMER_READ(child->startTime);

                pid = fork();
                if (pid < 0) {
                    perror("Failed to create new process.");
                    exit(EXIT_FAILURE);
                }

                if (pid > 0) {
                    child->pid = pid;
                    vector_pushBack(children, child);
                    runningChildren++;
                    printf("%s: background child started with PID %d.\n\n", COMMAND_RUN, pid);
                    continue;
                } else {
                    char seqsolver[] = "../CircuitRouter-SeqSolver/CircuitRouter-SeqSolver";
                    char *newArgs[4] = {seqsolver, args[1], args[2], NULL};

                    execv(seqsolver, newArgs);
                    perror("Error while executing child process"); // Nao deveria chegar aqui
                    exit(EXIT_FAILURE);
                }
            }

            else if (numArgs == 0){
                /* Nenhum argumento; ignora e volta a pedir */
                continue;
            }
            else
                printf("Unknown command. Try again.\n");

        }
    }

    if (fclose(fd) != 0) {
        perror("Error fclose");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < vector_getSize(children); i++) {
        free(vector_at(children, i));
    }
    vector_free(children);

    return EXIT_SUCCESS;
}
