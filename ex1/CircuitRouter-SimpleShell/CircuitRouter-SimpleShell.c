/*

TEST :
		run ../CircuitRouter-SeqSolver/inputs/random-x32-y32-z3-n64.txt


*/




#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "lib/commandlinereader.h"

#define MAXP 200
//int* status;




int varWait;
bool counterP;


typedef struct{
	pid_t p;
	int status;
} child_t;

typedef struct node{
	child_t* child;
	struct node *next;
}*listF;

void doRun(char** argVector, listF head, int MAX);
void parseArgs (int MAXCHILDREN);

listF NEW (listF head, child_t* c) {
	listF new = (listF)malloc(sizeof(listF));
	new->child = c;
	new->next = head;
	return new;
}


listF insert(listF head, child_t* c) {

	listF new = (listF)malloc(sizeof(listF));
	new->child = c;
	new->next = head;
  	return new;
}


void deleteL (listF head) {
	listF f, t = head;
	
	while(t->next != NULL) {
		f = t;
		t = t->next;
		free(f);
	}
	free(t);

}

void print (listF head) {
	listF elm;
	for (elm = head; elm != NULL; elm = elm->next){
		printf("CHILD EXITED (PID =%d; return %s)\n", elm->child->p, (elm->child->status) == 0 ? "OK" : "NOK");
	}

	puts("END");
}

int parseMAX(int argc, char const* arg) {
	int maxChildren;

	if (argc > 2){
		perror(" argumento longo");
		exit(EXIT_FAILURE);
	}



	if (argc == 1) {
		maxChildren = MAXP;
	}

	if (argc == 2){
		if(sscanf(arg, "%d", &maxChildren) < 0){
			perror("arg errado");
			exit(EXIT_FAILURE);
		}
	}

	return maxChildren;
}





void doRun(char** argVector, listF head, int MAX) {
	pid_t pid;

	pid = fork();

	if (pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	else if (pid == 0) {	//child process
		
		if (execl("../CircuitRouter-SeqSolver/seq", "../CircuitRouter-SeqSolver/seq",  argVector[1], NULL) == -1) {
			puts("error execl");
			exit(1);
			}
		exit(1);

		}



	else if (pid > 0) {	//parent process
		varWait = true;
		
		
		if (++counterP < MAX)
			parseArgs(MAX);

		if (counterP == MAX) {
			child_t* c;
      		c = (child_t*)malloc(sizeof(child_t));
      		c->p = wait(&c->status);
      		head = insert(head,c);

			counterP--;
			varWait = false;
		}

			
	}

	else {	//no child is created
		perror("No child created");
		exit(EXIT_FAILURE);
		}
	
}



void parseArgs (int MAXCHILDREN) {
	int numArgs;

	listF head = NULL;

	int buffersize = 256;
	char* buffer = (char*)malloc(64*sizeof(char));
	char** inputAux = (char**)malloc(64*sizeof(char*));

	if(varWait == true) {
		child_t* c;
      		c = (child_t*)malloc(sizeof(child_t));
      		c->p = wait(&c->status);
      		head = insert(head,c);

		counterP--;
	}

	numArgs = readLineArguments(inputAux,3, buffer, buffersize);


		if (numArgs == 2 && !strcmp(inputAux[0], "run")) {
			doRun(inputAux, head, MAXCHILDREN);
		}

		else if (numArgs == 1 && !strcmp(inputAux[0], "exit")) {
			print(head);
			exit(EXIT_SUCCESS);
		}

		else {
			puts("Argumento invalido");
			exit(EXIT_FAILURE);
		}

	free(inputAux);
	free(buffer);
	deleteL(head);	
}





int main (int argc, char** argv) {
	
	


	int MAXCHILDREN = parseMAX(argc, argv[1]);
	
	parseArgs(MAXCHILDREN);
	
	




	return 0;
}