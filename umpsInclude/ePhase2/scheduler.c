/* Questo modulo implementa lo scheduler dei processi di pKaya e il rivelatore dei deadlock */

/* GRUPPO: Alessandro Calo' - Alberto Scarlino - Manuela Corina - Antonello Antonacci */


#include <../umpsInclude/ePhase1/tcb.e>
#include <../umpsInclude/ePhase2/nucleo.e>
#include <../umpsInclude/libumps.h>
#include <../umpsInclude/ePhase2/scheduler.e>
#include <../umpsInclude/ePhase2/printTest.e>

/*
  Gestione dello scheduler.
  @return void.
*/

void scheduler () {
  int i;

  for(i=0;i<NUM_CPU;i++){
  /* Se esiste un processo in esecuzione */  
  if (CurrProc[i] != NULL){
    
    /*Imposta il tempo di partenza del processo sulla CPU*/
    CurrProc[i]->timeCpu += (GET_TODLOW - INTIME[i]);
    INTIME[i] = GET_TODLOW;
    
    /* Aggiorna il tempo trascorso dello pseudo-clock tick */
    tickTimer[i] += (GET_TODLOW - timerS[i]);
    timerS[i] = GET_TODLOW;
    
    /* Imposta l'Interval Timer col tempo minore rimanente tra il timeslice e lo pseudo-clock tick */
    SET_IT(MIN((SCHED_TIME_SLICE - CurrProc[i]->timeCpu), (SCHED_PSEUDO_CLOCK - tickTimer[i])));
    
    /* Carica lo stato del processo corrente */
    LDST(&(CurrProc[i]->p_state));
    
  }
   /* Se non sono presenti processi */
  else if (CurrProc[i] == NULL) {
 
    /* Verifica se la ReadyQueue é vuota */
    if (mkEmptyProcQ(&ReadyQueue[i])){
      if (processCounter[i] == 0) HALT(); 		/* Arresto del sistema */
      if  ((processCounter[i] > 0) && (soft_blockCounter[i] == 0)) PANIC();			/* Deadlock */ 
      if ((processCounter[i] > 0) && (soft_blockCounter[i] > 0)) { 		/* Stato di attesa */

	/* Settaggio stati con Interrupt attivati e non mascherati */
	setSTATUS ((getSTATUS() | STATUS_IEc | STATUS_INT_UNMASKED));
	while (TRUE);
          }
      PANIC();
    }
      
    /* Prende il primo processo pronto */  
    CurrProc[i] = removeProcQ(&ReadyQueue[i]);
    
    if (CurrProc[i] == NULL) PANIC();
    
    /* Calcola i millisecondi trascorsi dall'avvio dello pseudo-clock tick */ 
    tickTimer[i] += GET_TODLOW - timerS[i];
    timerS[i] = GET_TODLOW;
    
    /* Imposta il tempo di partenza del processo sulla CPU */
    CurrProc[i]->timeCpu = 0;
    INTIME[i] = GET_TODLOW;
    
    /* Imposta l'Interval Timer col tempo minore rimanente tra il timeslice e lo pseudo-clock tick */
    SET_IT(MIN(SCHED_TIME_SLICE, (SCHED_PSEUDO_CLOCK - tickTimer[i])));
    
    /* Carica lo stato del processo corrente */
    LDST(&(CurrProc->p_state));
  }
  PANIC();
}
}
