/* Questo modulo implementa il main() ed esporta le variabili globali del nucleo. */

/* GRUPPO: Alessandro Calo' - Alberto Scarlino - Manuela Corina - Antonello Antonacci */

#include <../umpsInclude/ePhase1/asl.e>
#include <../umpsInclude/ePhase1/pcb.e>
#include <../umpsInclude/ePhase2/libumps.e>   
#include <../umpsInclude/ePhase2/scheduler.e>
#include <../umpsInclude/ePhase2/wait.e>
#include <../umpsInclude/ePhase2/interrupts.e>
#include <../umpsInclude/ePhase2/syscall.e>
#include <../umpsInclude/const11.h>


/* Dichiarazione della lista dei pcb liberi e della lista dei pcb in attesa di I/O */
struct list_head readyQueue[NUM_CPU];
struct list_head WaitIOList[NUM_CPU];

/* vettore delle chiavi disponibili per i semafori */
int chiavi[11];

/* tempo di attesa della CPU */
U32 todProc;

/*Contatore dei processi */
U32 processCounter[NUM_CPU];

/* Contatore dei processi bloccati in attesa di I/O */
U32 soft_blockCounter[NUM_CPU];

/* Dichiarazione esterna della funzione test() */
extern void test();

/* Semaforo pseudo-clock timer */
int pseudo_clock[NUM_CPU];

/* Tempo trascorso del processo sullo pseudo_clock */
int tickTimer[NUM_CPU];

/* Tempo iniziale di utilizzo del processo e timer di Utilizzo della CPU stessa */
cpu_t INTIME[NUM_CPU], timerS[NUM_CPU];

/* Processo corrente */
pcb_t *currProc[NUM_CPU];


/* Funzione che popola le New Areas
   @param area : indirizzo dell'area da popolare
   @param gestore : inidirizzo del gestore dell'area
   @return void.   */
HIDDEN void popolaArea (memaddr area, memaddr gestore) {

    /* Nuova area da popolare */
    state_t *newArea;
    
    /* La nuova area punta alla vecchia */
    newArea = (state_t *) area;
    
    /* Salva lo stato corrente del processore */
    STST (newArea);
    
    /* Settaggio dell'indirizzo dei gestori delle varie eccezioni */
    newArea->pc_epc = newArea->reg_t9 = gestore;
    newArea->reg_sp = RAMTOP;
    
    /* Interrupt mascherati, Memoria Virtuale spenta, Kernel Mode attivo */
    newArea->status &= ~(STATUS_IEp | STATUS_VMp | STATUS_KUp);
}


/* Inizializzazione del nucleo
   @param  void
   @return void.   */
int main(void) {
	
    int plt[NUM_CPU];
    int i;
    int k;
    pcb_t *pstate[NUM_CPU];
	
    state_t *new_old_areas [NUM_CPU];
    
	
    /* Popolazione delle 4 nuove aree nella ROM Reserved Frame */

    /* Nuova area da popolare */    
    state_t *newArea[NUM_CPU];
    
    /*Inizializazione delle chiavi dei semafori*/
    for(k=0;k<11;k++){
		chiavi[k]=0;}

    for(i=0;i<NUM_CPU;i++){
	/*	Interrupt Exception Handling	*/
	popolaArea (INT_NEWAREA + (i * DIM * 8), (memaddr) interrupts);
    
	/*	TLB Exception Handling		*/
	popolaArea (TLB_NEWAREA + (i * DIM * 8), (memaddr) tlbtrap); 
    
	/*	PgmTrap Exception Handling	*/
	popolaArea (PGMTRAP_NEWAREA + (i * DIM * 8), (memaddr) prgtrap); 
    
	/*	SYS/BP Exception Handling	*/  
	popolaArea (SYSBK_NEWAREA + (i * DIM * 8), (memaddr) sysBpHandler);
     
 
	/* La nuova area punta alla vecchia */
	newArea[i] = (state_t *) SYSBK_NEWAREA;
     
	/* Salva lo stato corrente del processore */
	STST (newArea[i]);
     
	/* Interrupt attivati e mascherati, Memoria Virtuale spenta, Kernel-Mode spento */
	newArea[i]->status &= ~(STATUS_IEp | STATUS_VMp | STATUS_KUp);
    
	/* PC inizializzato all'indirizzo di syscallHandler() */
	newArea[i]->pc_epc = newArea[i]->reg_t9 = (memaddr) sysBpHandler;
    
	/* Il registro SP viene inizializzato a RAMTOP */
	newArea[i]->reg_sp = RAMTOP;
	new_old_areas[i]=newArea[i];
    }
	 
    /* Inizializzazione delle strutture dati del livello 2 (phase1) */
    initPcbs();
    initASL();
    
    /* Inizializzazione delle variabili globali contatore processi attivi, contatore processi bloccati I/O, timer pseudo_clock */ 
    for (i=0;i<NUM_CPU;i++){		
	processCounter[i] = soft_blockCounter[i] = pseudo_clock[i] = 0;
    }	
	
	
    /* Inizializzazione dei semafori dei device */
    for(i=0; i<DEV_PER_INT; i++)
    {
	sem.disk[i] = 0;
	sem.tape[i] = 0;
	sem.network[i] = 0;
	sem.printer[i] = 0;
	sem.terminalR[i] = 0;
	sem.terminalT[i] = 0;
    }
	
    /* Inizializzazione del Thread di gestione del test */
    /* Se il primo processo (init) non viene creato, PANIC() */
    
    for (i=0; i<NUM_CPU; i++){ 
    
    
	/* Inizializzazione delle liste di gestione dei Thread */
	mkEmptyProcQ(&readyQueue[i]); /* Thred Pronti */
	mkEmptyProcQ(&WaitIOList[i]); /* Thread in attesa */
    
   
	/* Allocazione spazio di memoria per il pcb */
	pstate[i] = allocPcb();
    
	/* Controlliamo che l'inizializzazione degli stati sia andata a buon fine */  
	if (pstate[i] == NULL) PANIC();
    
	/* Salviamo lo stato attuale della CPU */
	STST (&(pstate[i]->p_s));
				
	/* Interrupt attivati smascherati, Memoria Virtuale spenta, Kernel-Mode attivo process local timer */
	pstate[i]->p_s.status |= STATUS_INT_UNMASKED;
	pstate[i]->p_s.status |= STATUS_KUc;
	pstate[i]->p_s.status &= ~STATUS_VMp;
    
	
	/* Il registro SP viene inizializzato a RAMTOP - FRAMESIZE (traslato di due per evitare sovrapposizione nelle aree si memoria */
	pstate[i]->p_s.reg_sp = RAMTOP - (2 * FRAME_SIZE); 
    
	if(i==0){
           /* PC inizializzato all'indirizzo */
           pstate[i]->p_s.pc_epc = pstate[i]->p_s.reg_t9 = (memaddr)test;
	}
	else {
           pstate[i]->p_s.pc_epc = pstate[i]->p_s.reg_t9 = (memaddr)wait; 
	}
				
   
	/* inizializiamo il time slice dello sceduler */
	plt[i]=SCHED_TIME_SLICE;	
	INTIME[i]=0;
	tickTimer[i]=0;
    
     
	/* Inizzializzazione delle CPU */
	if (i < NUM_CPU - 1) {
	    INITCPU (i+1, &pstate[i], &new_old_areas[i]);
	}
	
	
	/* Inseriamo i processi nella coda di processi Ready */
	insertProcQ(&readyQueue[i], pstate[i]);
 
	/* Incremento contatore processi pronti */	
	processCounter[i]++;
	
	timerS[i]=GET_TODLOW;
	
	currProc[i]=NULL;
	
	if(i>0)LDST(&(pstate[i]->p_s));
    
    }
            
    /* Chiamata a scheduler() */     
    scheduler();
	
    return 0;
	
}


