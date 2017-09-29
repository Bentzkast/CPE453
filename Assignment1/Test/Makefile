CC = gcc
CFLAGS = -Wall -g -c

all: tryme.o
	$(CC) -L ../lib64 -o tryme tryme.o -lmalloc

tryme.o: tryme.c
	$(CC) $(CFLAGS) -o tryme.o tryme.c
