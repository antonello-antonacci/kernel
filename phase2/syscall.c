/* Il seguente file gestisce l'handler delle syscall delle PrgTrap e delle TlbTrap*/

/* GRUPPO: Alessandro Calo' - Alberto Scarlino - Manuela Corina - Antonello Antonacci*/


#include <initial.e>
#include <interrupts.e>
#include <scheduler.e>
#include <libumps.e>
#include <../umpsInclude/const.h>
#include <asl.e>
#include <pcb.e>
#include <../umpsInclude/ePhase2/printTest.e>


/* indice di sblocco della test&set */
int lock = 0;


/* Old Area delle Syscall */
HIDDEN state_t *sysOld = (state_t *) SYSBK_OLDAREA;
HIDDEN state_t *pgmOld = (state_t *) PGMTRAP_OLDAREA;
HIDDEN state_t *tlbOld = (state_t *) TLB_OLDAREA;


/* funzione di assegnazione del turno tra V e P nei semafori bloccati 
   @param  x, y : chiavi dei semafori da gestire
   @return void.  */
void ts (int x, int y) {
  
    y = x;
    x = 1;
}
  

/* Funzione che salva lo stato del processo nella Old area o viceversa 
   @param  current, old : inidirizzo della nuova e della vecchia area
   @return void.  */
void saveState (state_t *current, state_t *old) {
  
	int i;

	old->entry_hi = current->entry_hi;
	old->cause = current->cause;
	old->status = current->status;
	old->pc_epc = current->pc_epc;
	old->hi = current->hi;
	old->lo = current->lo;

	for (i = 0; i < 29; i++)
		old->gpr[i] = current->gpr[i];
}


/* Funzione che permette di creare un nuovo processo da aggiungere a quelli già esistenti 
   @param  statep : state da assegnare al nuovo processo creato
   @param  prty: priority da assegnare al nuovo processo
   @return (-1) se la creazione fallisce, (0) altrimenti  */
int createProcess(state_t *statep, int prty) {
  
    pcb_t *p;
    
    
    /* Se la allocPcb non va a buon fine restituisce -1 (errore) */
    if((p = allocPcb()) == NULL) return -1;
    
    else {
      
	/* Carica lo stato del processore in quello del processo */
	saveState(statep, &(p->p_s));
	p->p_s.status = STATUS_INT_UNMASKED;
	
	/* Aggiorna il contatore dei processi */
	processCounter[isOn_nCPU]++;

	/* Assegna al processo la priority */
	p->priority = prty;
	
	/* e inseriamo p come figlio del processo chiamante e nella lista dei pcb pronti */
	insertChild(currProc[isOn_nCPU], p);
	insertProcQ(&readyQueue[isOn_nCPU], p);
	
	return 0;
    }
    
}

/* Funzione che permette di creare un processo fratello di quello che richiede tale operazione 
   @param  statep : state da assegnare al nuovo processo creato
   @param  prty: priority da assegnare al nuovo processo
   @return (-1) se la creazione fallisce, (0) altrimenti  */
int createBrother(state_t *statep, int prty) {
  
    pcb_t *p;
    
    /* Se la allocPcb non va a buon fine restituisce -1 (errore) */
    if((p = allocPcb()) == NULL) return -1;
    else {
      
	/* Carica lo stato del processore in quello del processo */
	saveState(statep, &(p->p_s));

	/* Aggiorna il contatore dei processi */
	processCounter[isOn_nCPU]++;

	/* Assegna al processo la priority */
	p->priority = prty;
	
	/* e inseriamo p come fratello del processo chiamante ordinandolo in base alla sua priority */
	insertProcQ(&currProc[isOn_nCPU]->p_sib, p);
	
	return 0;
    }
}

/* Funzione che termina il processo corrente 

   @param  cProc : indirizzo del processo da terminare
   @return void  */
void terminateProcess(pcb_t *cProc) {
  
    pcb_t *pTerminate;
    pcb_t *pChild;
    	
    /* Se il currentProcess fosse NULL, ci troviamo in una condizione non stabilita quindi daremo un errore */
    if(cProc == NULL) PANIC();
    else {
	/* Altrimenti terminiamo tutti i processi figli di quello corrente */
	list_for_each_entry(pChild, &cProc->p_child, p_sib){
	  
	    pChild = removeChild(cProc);
	    
	    /* decrementiamo il contatore dei processi */
	    processCounter[isOn_nCPU]--;
	    removeProcQ(&readyQueue[isOn_nCPU]);
	    /* e liberiamo lo spazio di memoria */
	    freePcb(pChild);
	  
	}
	
	/* fatto ciò controlliamo se il processo è bloccato su qualche semaforo. In tal caso lo eliminiamo dal semaforo stesso */
	if (cProc->isBlocked) pTerminate = outBlocked(cProc);
	
	/* altrimenti provvediamo alla terminazione del processo corrente */
	pTerminate = outProcQ(&readyQueue[isOn_nCPU], cProc);
	processCounter[isOn_nCPU]--;
	removeProcQ(&readyQueue[isOn_nCPU]);
	currProc[isOn_nCPU]=NULL;
	freePcb(pTerminate);
    }
}


/* Funzione che genera una V sul semaforo specificato 
   @param  semkey : chiave del semaforo su cui fare la V
   @return void  */
void verhogen(int semkey) {
  
    pcb_t *p;
    int vp=1;
    
    /* si effettua un ciclo infinito per assegnare correttamente in turno al semaforo */
    do {
      
	vp=lock;
	lock=1;
      
    } while (vp);
	
    /* incrementiamo il valore del semaforo */
    chiavi[semkey]++;
    if (chiavi[semkey] <= 0){

	/* rimuoviamo p dalla coda dei processi bloccati (aggiornando automaticamente il flag isBlocked) */
	p = removeBlocked(semkey);
	removeProcQ (&WaitIOList[isOn_nCPU]);
	soft_blockCounter[isOn_nCPU]--;
	
	/* se non ci sono errori nella rimozione aggiungiamo p nella readyQueue */
	if (p != NULL) {
	    insertProcQ(&readyQueue[isOn_nCPU], p);
	    processCounter[isOn_nCPU]++;
	}
    }
    
    /* sblocchiamo il semaforo e chiamiamo lo scheduler */
    lock = 0;
    scheduler();
  
}   


/* Funzione che genera una P sul semaforo specificato 
   @param  semkey : chiave del semaforo su cui fare la p
   @return void */
void passeren (int semkey) {
  
    int ins;
    int vp=1;
    
    /* si effettua un ciclo infinito per assegnare correttamente in turno al semaforo */
    do {
	
	vp=lock;
	lock=1;
    } while (vp);
	
    /* decrementiamo il valore del semaforo */
    chiavi[semkey]--;
    if (chiavi[semkey] < 0){
		
	/* Inseriamo il processo corrente in coda al semaforo specificato verificando che tutto sia andato correttamente */
	ins = insertBlocked(semkey, currProc[isOn_nCPU]);
	insertProcQ(&WaitIOList[isOn_nCPU], currProc[isOn_nCPU]);
	soft_blockCounter[isOn_nCPU]++;

	if(ins == TRUE) PANIC();
    
	/* Aggiorniamo il tempo di vita del processo sulla CPU */
	currProc[isOn_nCPU]->time_cpu += (GET_TODLOW - todProc);
	todProc = GET_TODLOW;
		
	currProc[isOn_nCPU] = NULL;
    }
    
    /* sblocchiamo il semaforo */
    lock = 0;
    
}



/* Funzione che restituisce il tempo di vita del processo 
   @param  void
   @return void  */
cpu_t getCPUTime() {
  
	/* Aggiorna il tempo di vita del processo sulla CPU */
	currProc[isOn_nCPU]->time_cpu += (GET_TODLOW - todProc);
	todProc = GET_TODLOW;
	
	return currProc[isOn_nCPU]->time_cpu;
}


/* Funzione che mette in attesa lo pseudo_clock per 100 millisecondi (la V sbloccherà automaticamente lo pseudo_clock) 
   @param  void
   @return void */
void waitClock() {
  
    int ins;
    int vp;
    
    /* eseguiamo un ciclo infinito per rimanere in attesa di un segnale di sblocco da parte dello pseudo_clock */
    while (TRUE){
      
	/* si effettua un ciclo infinito per assegnare correttamente in turno al semaforo */
	do {
	    ts(lock, vp);
	} while (vp);
    
	/* decremntiamo il valore dello pseudo_clock */
	pseudo_clock[isOn_nCPU]--;

	if(pseudo_clock[isOn_nCPU] < 0) {
      
	    /* Inseriamo il processo corrente in coda al semaforo specificato */
	    ins = insertBlocked(pseudo_clock[isOn_nCPU], currProc[isOn_nCPU]);
	    if(ins == TRUE) PANIC();
	
	    /* e aggiorniamo il tempo di vita del processo */
	    currProc[isOn_nCPU]->time_cpu += (GET_TODLOW - todProc);
	    todProc = GET_TODLOW;
		
	    /* spostiamo il controllo ad un'altra cpu, in attesa che lo pseudo_clock sblocchi questo processo */
	    currProc[isOn_nCPU] = NULL;
	}
	
	lock = 0;
    }
}

/* Funzione che mette in attesa di I/O un determinato processo 
   @param  intNo: linea di interrupt del device
   @param  dnum: numero del device
   @param  waitForTermRead: indice di lettura o scrittura del device
   @return stato del device  */
unsigned int waitIO(int intNo, int dnum, int waitForTermRead) {
    
    /* tramite lo switch sulla linea di interrupt capiamo di quale device siamo in attesa */
    switch(intNo) {
	/* una volta scoperto il device effettuiamo una P per permettere il blocco del processo */
	/* provvediamo inoltre ad incrementare il numero di processi bloccati */
	case INT_DISK:
	    passeren((int) sem.disk[dnum]);
	    soft_blockCounter[isOn_nCPU]++;
	break;
	
	case INT_TAPE: 
	    passeren((int) sem.tape[dnum]);
	    soft_blockCounter[isOn_nCPU]++;
	break;
	
	case INT_UNUSED: 
	    passeren((int) sem.network[dnum]);
	    soft_blockCounter[isOn_nCPU]++;
	break;
	
	case INT_PRINTER: 
	    passeren((int) sem.printer[dnum]);
	    soft_blockCounter[isOn_nCPU]++;
	break;
	
	case INT_TERMINAL: 
	    if(waitForTermRead) {
		passeren((int) sem.terminalR[dnum]);
		soft_blockCounter[isOn_nCPU]++;
	    }
	    else {
		passeren((int) sem.terminalT[dnum]);
		soft_blockCounter[isOn_nCPU]++;
	    }
	break;
	
	default: 
	    PANIC();
	
    }
    
    return DEV_REGS_START + (dnum * DEV_REG_SIZE);
}


/* Funzione di gestione della Sys 9 che permette di specificare l'area su cui il kernel effettua una eccezione
   di tipo pmgTrap. Se ciò avviene, viene settato il flag pgmMake.
 
   @param  oldArea: vecchia area del pgm handler
   @param  newArea: nuova area del pgm handler
   @return void  */

void specPrgVec(state_t *oldArea, state_t *newArea) {

    currProc[isOn_nCPU]->pgmOldArea = oldArea;
    currProc[isOn_nCPU]->pgmNewArea = newArea;
    currProc[isOn_nCPU]->pgmMake = 1;
}


/* Funzione di gestione della Sys 10 che permette di specificare l'area su cui il kernel effettua una eccezione
   di tipo tlbTrap. Se ciò avviene, viene settato il flag tlbMake 
   
   @param  oldArea: vecchia area del tlb handler
   @param  newArea: nuova area del tlb handler
   @return void  */
void specTlbVec(state_t *oldp, state_t *newp) {

    currProc[isOn_nCPU]->tlbOldArea = oldp;
    currProc[isOn_nCPU]->tlbNewArea = newp;
    currProc[isOn_nCPU]->tlbMake = 1;
}

/* Funzione di gestione della Sys 11 che permette di specificare l'area su cui il kernel effettua una eccezione
   di tipo syscall 
   
   @param  oldp: vecchia area delle syscall
   @param  newp: nuova area delle syscall
   @return void  */
void specSysVec(state_t *oldp, state_t *newp) {

    /* assegnamo al processo le vecchie aree delle syscall */
    currProc[isOn_nCPU]->sysOldArea = oldp;
    currProc[isOn_nCPU]->sysNewArea = newp;
}

/* Gestore delle prgtrap */
void prgtrap(){
  
    /* per prima cosa salviamo lo state del processo corrente nella OldArea delle pgmTrap */
    if (&currProc[isOn_nCPU] != NULL) saveState(pgmOld, &currProc[isOn_nCPU]->p_s);
    
    /* Se non è già stata effettuata la sys 9 terminiamo il processo e chiamiamo lo scheduler */
    if (currProc[isOn_nCPU]->pgmMake != 1) {
	terminateProcess (currProc[isOn_nCPU]);
	scheduler();
    }
    /* altrimenti settiamo lo stato nella OldArea del processo (dedicata alle pgmTrap e carichiamo tale stato per l'esecuzione */
    else {
	saveState(pgmOld, currProc[isOn_nCPU]->pgmOldArea);
	LDST (currProc[isOn_nCPU]->pgmNewArea);
    }

}

/* Gestore delle tlbtrap */
void tlbtrap(){
  
    /* per prima cosa salviamo lo state del processo corrente nella OldArea delle tlbTrap */
    if (&currProc[isOn_nCPU] != NULL) saveState(tlbOld, &currProc[isOn_nCPU]->p_s);
  
    /* Se non è già stata effettuata la sys 10 terminiamo il processo e chiamiamo lo scheduler */
    if (currProc[isOn_nCPU]->tlbMake != 1) {
	terminateProcess (currProc[isOn_nCPU]);
	scheduler();
    }
    /* altrimenti settiamo lo stato nella OldArea del processo (dedicata alle tlbTrap e carichiamo tale stato per l'esecuzione */
    else {
	saveState(tlbOld, currProc[isOn_nCPU]->tlbOldArea);
	LDST (currProc[isOn_nCPU]->tlbNewArea);
    }

}

/* Gestore delle syscall */
void sysBpHandler() {
    
    int cause_excCode;
    int ku_Mode;

    /* per prima cosa salviamo lo state del processo corrente nella OldArea delle Syscall */
    saveState(sysOld, &currProc[isOn_nCPU]->p_s);

    /* Per evitare ciclo infinito di Syscall spostiamo il registro epc in avanti di 4 */
    currProc[isOn_nCPU]->p_s.pc_epc += 4;

    /* Recupera il bit della modalità della sysBp Old Area (per controllare se USER MODE o KERNEL MODE) */
    ku_Mode = ((sysOld->status) & STATUS_KUp) >> 0x3;

    /* Recupera il tipo di eccezione avvenuta */
    cause_excCode = CAUSE_EXCCODE_GET(sysOld->cause);

    /* Controlla se è una syscall */
    if (cause_excCode == EXC_SYSCALL) {
	/* Controlla se è in USER MODE */
	if(ku_Mode == TRUE) {
	    /* Se è stata chiamata una delle Syscall */
	    if((sysOld->reg_a0 > 0) && (sysOld->reg_a0 < 11)) {
		/* Imposta Cause.ExcCode a RI */
		sysOld->cause = CAUSE_EXCCODE_SET(sysOld->cause, EXC_RESERVEDINSTR);
		/* Salva lo stato della SysBP Old Area nella pgmTrap Old Area */
		saveState(sysOld, pgmOld);
		prgtrap();
	    }
	    
	}
	/* Altrimenti la gestisce in KERNEL MODE */
	else {

	    /* Salva i parametri delle aree delle SYSCALL */
	    U32 regA1 = sysOld->reg_a1;
	    U32 regA2 = sysOld->reg_a2;
	    U32 regA3 = sysOld->reg_a3;
	    
	    /* Gestisce ogni singola SYSCALL */
	    switch(sysOld->reg_a0) {
		case CREATEPROCESS:
		    currProc[isOn_nCPU]->p_s.reg_v0 = createProcess((state_t *) regA1, (int) regA2);
		break;

		case CREATEBROTHER:
		    currProc[isOn_nCPU]->p_s.reg_v0 = createBrother((state_t *) regA1, (int) regA2);
		break;

		case TERMINATEPROCESS:
		    terminateProcess(currProc[isOn_nCPU]);
		break;

		case VERHOGEN:
		    verhogen((int) regA1);
		break;

		case PASSEREN:
		    passeren((int) regA1);
		break;

		case GETCPUTIME:
		    currProc[isOn_nCPU]->p_s.reg_v0 = getCPUTime();
		break;

		case WAITCLOCK:
		    waitClock();
		break;

		case WAITIO:
		    currProc[isOn_nCPU]->p_s.reg_v0 = waitIO((int) regA1, (int) regA2, (int) regA3);
		break;

		case SPECPRGVEC:
		    specPrgVec((state_t *) regA1, (state_t *) regA2);
		break;

		case SPECTLBVEC:
		    specTlbVec((state_t *) regA1, (state_t *) regA2);
		break;

		case SPECSYSVEC:
		    specSysVec((state_t *) regA1, (state_t *) regA2);
		break;

		default:
		    /* Salva la SysBP Old Area all'interno del processo corrente */
		    saveState(sysOld, currProc[isOn_nCPU]->sysOldArea);
		    LDST(currProc[isOn_nCPU]->sysNewArea);}
	      
			
	      scheduler();
	}
    }
    /* Chiamata di una Syscall non esistente */
    else PANIC();
}

