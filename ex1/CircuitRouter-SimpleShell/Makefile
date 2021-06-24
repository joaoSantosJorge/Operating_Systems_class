CFLAGS =-Wall -std=gnu99 -I../
CC = gcc

shell: CircuitRouter-SimpleShell.o ../lib/commandlinereader.o
	$(CC) $(CFLAGS) -o shell CircuitRouter-SimpleShell.o ../lib/commandlinereader.o


CircuitRouter-SimpleShell.o: CircuitRouter-SimpleShell.c ../lib/commandlinereader.o
	$(CC) $(CFLAGS) -c CircuitRouter-SimpleShell.c -o CircuitRouter-SimpleShell.o

commandlinereader.o: ../lib/commandlinereader.c ../lib/commandlinereader.h
	$(CC) $(CFLAGS) -c ../lib/commandlinereader.c -o ../lib/commandlinereader.o

clean:
	rm -f *.o Shell

run:
	./shell
