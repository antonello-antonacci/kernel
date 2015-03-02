# Makefile di phase1

# Dichiarazione delle cartelle base
INCLUDE = ../umpsInclude/
INCLUDEPATHE = ../umpsInclude/ePhase1/
LIB = ../supporto/

# Dichiarazione dei comandi base
CFLAGS = -Wall -I $(INCLUDE) -I $(INCLUDEPATHE) -I $(LIB) -c
CC = mipsel-linux-gcc


# Target principale
 all: pcb.o asl.o 
#printTest.o
# Per testare phase1, commentare la riga precedente e decommentare le sottostanti
#all: pcb.o asl.o p1test_pkaya_v1.1.o printTest.o

pcb.o: pcb.c
	$(CC) $(CFLAGS) pcb.c

asl.o: asl.c
	$(CC) $(CFLAGS) asl.c

#p1test_pkaya_v1.0.o: p1test_pkaya_v1.1.c
#	$(CC) $(CFLAGS) p1test_pkaya_v1.1.c

#printTest.o: printTest.c
#	$(CC) $(CFLAGS) printTest.c

# Pulizia dei file oggetto
clean:
	rm -f *.o
