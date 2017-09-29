CC = gcc

CFLAGS = -Wall -Werror

dine: dine.o
	$(CC) $(CFLAGS) -lpthread -lrt -o $@ dine.o

dine.o: dine.c
	$(CC) $(CFLAGS) -c -o dine.o dine.c

clean:
	rm -f dine.o dine
