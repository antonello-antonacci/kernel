# Makefile per mips-linux

# Dichiarazione delle cartelle base# Makefile per mips-linux

# Dichiarazione delle cartelle base
INCLUDE = ./umpsInclude
PHASE1 = ./phase1
PHASE2 = ./phase2
INCLUDEPATHE1 = ./umpsInclude/ePhase1
INCLUDEPATHE2 = ./umpsInclude/ePhase2
SUPDIR = ./supporto


# Dichiarazione dei comandi base
  

CFLAGS = -Wall -I $(INCLUDE) -I $(INCLUDEPATHE1) -I $(INCLUDEPATHE2) -I $(SUPDIR) -ansi -pedantic -c
LDFLAGS =  -T 
CC = mipsel-linux-gcc
LD = mipsel-linux-ld

# Target principale
all: kernel.core.umps2 tape0.umps2 disk0.umps2

tape0.umps2: kernel.core.umps
	umps2-mkdev -t $@ $<

disk0.umps2: kernel.core.umps
	umps2-mkdev -d $@ 

kernel.core.umps2: kernel
	umps2-elf2umps -k kernel


# Linking del kernel

kernel: phase1dir phase2dir $(SUPDIR)/crtso.o $(SUPDIR)/libumps.o
	$(LD) $(LDFLAGS) 	$(SUPDIR)/elf32ltsmip.h.umpscore.x \
				$(SUPDIR)/crtso.o \
				$(SUPDIR)/libumps.o \
				$(PHASE1)/asl.o \
				$(PHASE1)/pcb.o \
				$(PHASE2)/initial.o \
				$(PHASE2)/scheduler.o \
				$(PHASE2)/interrupts.o \
				$(PHASE2)/syscall.o \
				$(PHASE2)/printTest.o \
				$(PHASE2)/wait.o \
				$(PHASE2)/p2test2012_v1.0.o \
				-o kernel

# Sorgenti di phase1
phase1dir:
	cd $(PHASE1) && make all

# Sorgenti di phase2
phase2dir:
	cd $(PHASE2) && make all


# Pulizia parziale dei file creati
clean:
	rm -f *.o kernel
	rm -f kernel.*.umps2
	rm -f term*.umps2 printer*.umps2 tape0.umps2 disk0.umps2

# Pulizia totale dei file creati
cleanall:
	rm -f *.o kernel
	rm -f kernel.*.umps2
	cd $(PHASE1) && make clean
	cd $(PHASE2) && make clean
