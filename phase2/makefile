# Makefile di phase2

# Dichiarazione delle cartelle base
INCLUDE = ../umpsInclude/
PHASE1 = ../phase1/
INCLUDEPATHE2 = ../umpsInclude/ePhase2/
INCLUDEPATHE1 = ../umpsInclude/ePhase1/
SUPDIR = ../supporto/

# Dichiarazione dei comandi base
CFLAGS = -Wall -I $(INCLUDE) -I $(PHASE1) -I $(INCLUDEPATHE1) -I $(INCLUDEPATHE2)  -I $(SUPDIR) -c
CC = mipsel-linux-gcc

# Target principale

all: initial.o scheduler.o syscall.o interrupts.o wait.o  p2test2012_v1.0.o
#printTest.o

initial.o: initial.c
	$(CC) $(CFLAGS) initial.c

scheduler.o: scheduler.c
	$(CC) $(CFLAGS) scheduler.c

#printTest.o: printTest.c
#	$(CC) $(CFLAGS) printTest.c

syscall.o: syscall.c
	$(CC) $(CFLAGS) syscall.c

interrupts.o: interrupts.c
	$(CC) $(CFLAGS) interrupts.c

wait.o: wait.c
	$(CC) $(CFLAGS) wait.c
	

p2test2012_v1.0.o: p2test2012_v1.0.c
	$(CC) $(CFLAGS) p2test2012_v1.0.c


# Pulizia dei file oggetto
clean:
	rm -f *.o
