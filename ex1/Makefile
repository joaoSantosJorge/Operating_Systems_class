CFLAGS =-Wall -std=gnu99
CC = gcc

all:
	make -C CircuitRouter-SeqSolver
	make -C CircuitRouter-SimpleShell

clean:
	cd CircuitRouter-SeqSolver && make clean
	cd CircuitRouter-SimpleShell && make clean
	cd lib && rm -f *.o lib

seq:
	cd CircuitRouter-SeqSolver && make run
shell:
	cd CircuitRouter-SimpleShell && make run
