/*
 *		compilar:			gcc -Wall -pthread threadEx.c -o a
 *		correr prog:		./a
 *
*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define N 5
#define TAMANHO 10

int buffer [N][TAMANHO];
int nsomas;

void inicializaMatriz(void) {
	int i, j;

	for (i = 0; i < N; i++)
		for (j = 0; j < TAMANHO; j++)
			buffer [i][j] = rand()%10;
}


void *soma_linha (void *linha) {
	//sleep(5);
	int c, soma = 0;
	int *b = linha;

	for (c = 0; c < TAMANHO-1; c++) {
		soma += b[c];
		nsomas++;
	}

	b[c] = soma;
	int* ret = (int*)malloc(sizeof(int));
	*ret = soma;
	return ret;
}

void imprimeResultados(void) {
	int i, j;
	for (i = 0; i < N; i++) {
		puts("");
		for (j = 0; j < TAMANHO; j++)
			printf("[%d] ", buffer[i][j]);
	}
	puts("");
}

int main (void) {
	int i;
	pthread_t tid[N];
	void *results[N];

	inicializaMatriz();

	for (i = 0; i < N; i++) {
		if (pthread_create(&tid[i], 0, soma_linha, buffer[i]) == 0) {
			printf("Criada tarefa %d\n", (int) tid[i]);
		}
		else {
			printf("Erro na criacao de tarefa\n");
			return EXIT_FAILURE;
		}
	}

	for (i = 0; i < N; i++) {
		pthread_join(tid[i], &results[i]);		//segundo argumento e suposto ser NULL na maioria dos casos
		printf("A tarefa %d devolveu %p\n", i, &results[i] );
	}

	printf("termiraram todas as tarefas\n");

	imprimeResultados();

	return EXIT_SUCCESS;
}