/*
	./CircuitRouter-Client ../CircuitRouter-AdvShell/CircuitRouter-AdvShell.pipe
*/


#include "CircuitRouter-Client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "lib/commandlinereader.h"

#define PERMISSIONS 0777

#define MAXARGS 3
#define BUFFER_SIZE 100

int main (int argc, char** argv) {

	FILE* fd;
	char *args[MAXARGS+1];
	char buffer[BUFFER_SIZE];
	char* myfifo;

	if (argv[1] == NULL) {
		perror("Argument missing");
		exit(EXIT_FAILURE);
	}

	myfifo = argv[1];

	printf("Welcome to CircuitRouter-Client\n\n");

	if (access(myfifo, F_OK) == 0) {
		fd = fopen(myfifo, "a");
		if (fd == NULL) {
			perror("Error fopen");
			exit(EXIT_FAILURE);
		}
	}
	else {
		perror("Error access");
		exit(EXIT_FAILURE);
	}


	while(1) {
		char tempFile[] = "XXXXXX";
		if (mkstemp(tempFile) == -1) {
			perror("Error mkstemp");
			exit(EXIT_FAILURE);
		}

		if (unlink(tempFile) != 0) {
            perror("Error unlink");
            exit(EXIT_FAILURE);
        }
	    if (mkfifo(tempFile, PERMISSIONS) != 0) {
	        perror("Error mkfifo");
	        exit(EXIT_FAILURE);
	    }

		int numArgs = 0;

		numArgs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE, stdin);
		if(numArgs < 1) {
			printf("Invalid numArgs\n");
			exit(EXIT_FAILURE);
		}


		char pathToFile[] = "../CircuitRouter-Client/";
		strcat(pathToFile, tempFile);


		FILE* pipeRet;
		pipeRet = fopen(pathToFile, "r+");
		if (pipeRet == NULL) {
			perror("Error fopen");
			exit(EXIT_FAILURE);
		}

		fprintf(fd, "%s %s %s\n", args[0], args[1], pathToFile);
		fflush(fd);


		char str[50];
		fgets(str, 50, pipeRet);

		puts(str);

		if (fclose(pipeRet) != 0) {
			perror("Error fclose");
			exit(EXIT_FAILURE);
		}


	}

	if (fclose(fd) != 0) {
		perror("Error fclose");
		exit(EXIT_FAILURE);
	}



	return EXIT_SUCCESS;
}
