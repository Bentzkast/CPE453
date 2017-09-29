all: hello

test:hello
	@echo Testing hello...
	@./hello
	@echo done.
hello: hello.o
	gcc -g -o hello hello.o
hello.o: hello.c
	gcc -Wall -ansi -pedantic -g -c -o hello.o hello.c

clean:
	rm *.o
	rm hello
