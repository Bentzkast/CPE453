CC = gcc
CCFLAGS = -Wall -g -fPIC
all: lib/malloc.so 

intel-all: lib/libmalloc.so lib64/libmalloc.so

lib/libmalloc.so: lib malloc32.o
	$(CC) $(CCFLAGS) -m32 -shared -o $@ malloc32.o

lib64/libmalloc.so: lib64 malloc64.o 
	$(CC) $(CCFLAGS) -shared -o $@ malloc64.o

malloc32.o: malloc.c
	$(CC) $(CCFLAGS) -m32 -c -o malloc32.o malloc.c

malloc64.o: malloc.c 	
	$(CC) $(CCFLAGS) -c -o malloc64.o malloc.c

lib: 
	mkdir -p lib

lib64:
	mkdir -p lib64

clean:
	rm -f malloc.o *~  
