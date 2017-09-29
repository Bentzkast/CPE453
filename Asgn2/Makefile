CC 	= gcc

CFLAGS  = -Wall -g -I .

LD 	= gcc

LDFLAGS  = -Wall -g -L/home/pn-cs453/Given/Asgn2

PUBFILES =  README  hungrymain.c  libPLN.a  libsnakes.a  lwp.h\
	    numbersmain.c  snakemain.c  snakes.h

TARGET =  pn-cs453@hornet:Given/asgn2

PROGS	= snakes nums hungry

SNAKEOBJS  = snakemain.o 

HUNGRYOBJS = hungrymain.o 

NUMOBJS    = numbersmain.o

OBJS	= $(SNAKEOBJS) $(HUNGRYOBJS) $(NUMOBJS) 

SRCS	= snakemain.c numbersmain.c

HDRS	= 

EXTRACLEAN = core $(PROGS)

all: 	$(PROGS)

allclean: clean
	@rm -f $(EXTRACLEAN)

clean:	
	rm -f $(OBJS) lwp.o scheduler.o magic64.o *~ TAGS

snakes: snakemain.o liblwp.so libsnakes.a
	$(LD) -Wall -g -o snakes snakemain.o -L. -ltinfo -lncurses -lsnakes -llwp

hungry: hungrymain.o liblwp.so libsnakes.a scheduler.o
	$(LD) -Wall -g -o hungry hungrymain.o -L. -ltinfo -lncurses -lsnakes -llwp

nums: numbersmain.o liblwp.so
	$(LD) -Wall -g -o nums numbersmain.o -L. -llwp

scheduler.o : scheduler.h
	$(LD) -Wall -g -fPIC -c scheduler.c

hungrymain.o: lwp.h snakes.h

snakemain.o: lwp.h snakes.h

numbersmain.o: lwp.h numbersmain.c
	$(LD) -c numbersmain.c

liblwp.so : lwp.o magic64.o scheduler.o
	$(LD) -Wall -g -fPIC -shared -o liblwp.so lwp.o magic64.o scheduler.o

lwp.o : lwp.c lwp.h
	$(LD) -Wall -g -fPIC -c lwp.c

magic64.o : magic64.S
	$(LD) -o magic64.o -c magic64.S

pub:
	scp $(PUBFILES) $(TARGET)

