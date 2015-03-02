/* Questo modulo implementa lo scheduler dei processi di pKaya e il rivelatore dei deadlock */

/* GRUPPO: Alessandro Calo' - Alberto Scarlino - Manuela Corina - Antonello Antonacci */


#include <../umpsInclude/ePhase1/pcb.e>
#include <../umpsInclude/ePhase2/initial.e>
#include <../umpsInclude/ePhase2/scheduler.e>
#include <../umpsInclude/ePhase2/syscall.e>
#include <../umpsInclude/libumps.h>
#include <../umpsInclude/listx.h>


/* Dichiarazione del vettore di gestione temporaneo per i processi bloccati e dell'indicatore della CPU in utilizzo */
pcb_t *temp[NUM_CPU];
int isOn_nCPU;


/* Gestione dello scheduler.
   @param  void
   @return void. */
void scheduler () {

  int i;

  /* Per ogni CPU */
  for(i=0;i<NUM_CPU;i++){
  
      /* Se esiste un processo in esecuzione */  
      if (currProc[i] != NULL){
	
	  isOn_nCPU = i;
	  /*Imposta il tempo di partenza del processo sulla CPU*/
	  currProc[i]->time_cpu += (GET_TODLOW - INTIME[i]);
	  INTIME[i] = GET_TODLOW;
    
	  /* Aggiorna il tempo trascorso dello pseudo-clock tick */
	  tickTimer[i] += (GET_TODLOW - timerS[i]);
	  timerS[i] = GET_TODLOW;
    
	  /* Imposta l'Interval Timer col tempo minore rimanente tra il timeslice e lo pseudo-clock tick */
	  SET_IT(MIN((SCHED_TIME_SLICE - currProc[i]->time_cpu), (SCHED_PSEUDO_CLOCK - tickTimer[i])));
    
	  /* Carica lo stato del processo corrente */
	  LDST(&(currProc[i]->p_s));
     
      }
      /* Se non sono presenti processi */
      else if (currProc[i] == NULL) {
  
	  /* Verifica se la ReadyQueue é vuota */
	  if (emptyProcQ(&readyQueue[i])){
	
	  if (processCounter[i] == 0) HALT(); 	/* Se il contatore dei processi è 0 arresto del sistema */
	  if  ((processCounter[i] > 0) && (soft_blockCounter[i] == 0)) PANIC(); /* se il soft_block counter è 0 Deadlock */ 
	  if ((processCounter[i] > 0) && (soft_blockCounter[i] > 0)) { 	/* Altrimenti stato di attesa */
      
	      /* Settaggio stati con Interrupt attivati e non mascherati */
	      setSTATUS ((getSTATUS() | STATUS_IEc | STATUS_INT_UNMASKED));
         
	      /*verifico lo PseudoClock */
	      if ((( GET_TODLOW - tickTimer[i]) >= SCHED_PSEUDO_CLOCK))  { 	
				
		  /* faccio una v per tutti i processi bloccati in atessa di un pseudoclock Tick */
		  while((temp[i]=removeProcQ(&WaitIOList[i]))!=NULL){
		      verhogen(temp[i]->p_semkey);
		  }
			
		  /* resetto lo pseudoClock timer */ 
		  tickTimer[i] = GET_TODLOW;
         
	      }
	    PANIC();
	 }
	 }
      
	/* Prende il primo processo pronto */ 
	currProc[i] = removeProcQ(&readyQueue[i]);
	if (currProc[i] == NULL) PANIC();
    
	/* Calcola i millisecondi trascorsi dall'avvio dello pseudo-clock tick */ 
	tickTimer[i] += GET_TODLOW - timerS[i];
	timerS[i] = GET_TODLOW;
    
	/* Imposta il tempo di partenza del processo sulla CPU */
	currProc[i]->time_cpu = 0;
	INTIME[i] = GET_TODLOW;
    
	/* Imposta l'Interval Timer col tempo minore rimanente tra il timeslice e lo pseudo-clock tick */
	SET_IT(MIN(SCHED_TIME_SLICE, (SCHED_PSEUDO_CLOCK - tickTimer[i])));
    
	/* Carica lo stato del processo corrente */
	LDST(&(currProc[i]->p_s));
     
      }
    PANIC();
  }
}

