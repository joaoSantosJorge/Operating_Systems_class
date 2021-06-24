# Makefile Client

SOURCES = CircuitRouter-Client.c 
SOURCES+= ../lib/commandlinereader.c
OBJS = $(SOURCES:%.c=%.o)
CC   = gcc
CFLAGS =-Wall -std=gnu99 -I../
LDFLAGS=-lm
TARGET = CircuitRouter-Client

all: $(TARGET) CircuitRouter-Client

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LDFLAGS) 

CircuitRouter-Client.o: CircuitRouter-Client.c CircuitRouter-Client.h ../lib/commandlinereader.h
../lib/commandlinereader.o: ../lib/commandlinereader.c ../lib/commandlinereader.h

$(OBJS):
	$(CC) $(CFLAGS) -c -o $@ $<


clean:
	@echo Cleaning...
	rm -f $(OBJS) $(TARGET)
