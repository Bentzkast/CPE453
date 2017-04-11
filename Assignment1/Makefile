CC = gcc
CCFLAGS = -Wall -g -fPIC
SOURCES = $(malloc.c)
OBJECTS = $(SOURCES:.c=.o)

all: dir32 malloc32 

intel-all: dir32 dir64 malloc32 malloc64

malloc32: malloc32.o
	$(CC) $(CCFLAGS) -m32 -shared -o lib/libmalloc.so malloc32.o

malloc64: malloc64.o 
	$(CC) $(CCFLAGS) -shared -o lib64/libmalloc.so malloc64.o

malloc32.o: malloc.c
	$(CC) $(CCFLAGS) -m32 -c -o malloc32.o malloc.c

malloc64.o: malloc.c 	
	$(CC) $(CCFLAGS) -c -o malloc64.o malloc.c

dir32: 
	mkdir -p lib

dir64:
	mkdir -p lib64

clean:
	rm -f malloc.o *~  
