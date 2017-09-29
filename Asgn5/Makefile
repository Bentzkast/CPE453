all: minls minget

minixlib.o: minixlib.c minixlib.h
	gcc -Wall -g -c -o minixlib.o minixlib.c

minls.o: minls.c
	gcc -Wall -g -c -o minls.o minls.c

minget.o: minget.c
	gcc -Wall -g -c -o minget.o minget.c

minls: minls.o minixlib.o
	gcc -Wall -g minls.o minixlib.o -o minls

minget: minget.o minixlib.o
	gcc -Wall -g minget.o minixlib.o -o minget

clean:
	rm -f *.o
	rm -f minls
	rm -f minget