/* Il seguente file gestisce gli interrupts */

/* GRUPPO: Alessandro Calo' - Alberto Scarlino - Manuela Corina - Antonello Antonacci*/


#include <../umpsInclude/ePhase2/initial.e>
#include <../umpsInclude/ePhase2/interrupts.e>
#include <../umpsInclude/ePhase2/scheduler.e>
#include <../umpsInclude/ePhase2/libumps.e>
#include <../umpsInclude/const.h>
#include <../umpsInclude/ePhase1/asl.e>
#include <../umpsInclude/ePhase1/pcb.e>
#include <../umpsInclude/uMPStypes.h>
#include <../umpsInclude/const11.h>
#include <../umpsInclude/ePhase2/syscall.e>
#include <../umpsInclude/types11.h>

 
/* Funzione di gestione degli specifici device.
   @param linea: linea di interrupt sul quale è collocato il device
   @return numero del device da gestire.   */
unsigned int device(unsigned int linea){
	
  
    unsigned int deviceAdd,deviceRequest,terminalRcv,terminalTra ;
    unsigned int returnValue=0;
    unsigned int i=0;
        
    /* Calcolo dell' indirizzo attuale della bitmap sul device */
    deviceAdd = *((int*)(PENDING_BITMAP_START + (linea - 3)* WORD_SIZE));
	
    /* Cerco il numero del device che ha effetuato l'interrupt */
    while (i<= DEV_PER_INT && (deviceAdd%2 == 0)) {
	deviceAdd >>= 1;
	i++;
    }
	
    /* Ricavo il registro del device richiesto spostando il puntatore del vettore dei registri */
    deviceRequest = DEV_REGS_START + ((linea - 3) * 0x80) + (i * 0x10);  
	
    /* Ricavo device che ha generato l' interrupt */
    if(linea!=INT_TERMINAL){ 
      
	/* Non è sulla linea del terminale */
	((dtpreg_t*)deviceRequest)->command = DEV_C_ACK;

	/* Prendo il valore del registro status del device */
	returnValue= ( ((dtpreg_t*)deviceRequest)->status)& 0xFF;

    }
    else{  
		
	/* Altrimenti è il terminale */
	/* Prendo il valore dal campo recv_status */
	terminalRcv = ( ((termreg_t *)deviceRequest)->recv_status)& 0xFF;
	
	/* Prendo il valore dal campo transm_status */
	terminalTra = ( ((termreg_t *)deviceRequest)->transm_status)& 0xFF ;
		
	/* Controllo se il terminale è in fase di trasmissione */
	if (terminalTra == DEV_TTRS_S_CHARTRSM) {
	    ((termreg_t*)deviceRequest)->transm_command = DEV_C_ACK;
	    returnValue=terminalTra;
	}
	else{
		
	    /* oppure di ricezione */
	    if (terminalRcv == DEV_TRCV_S_CHARRECV) {
		((termreg_t*)deviceRequest)->recv_command = DEV_C_ACK;
		returnValue=terminalRcv;
	    }

	}
    }
    
    /* Restituisco il valore dello status */
    return returnValue;

}


/*Funzione di gestione degli interrupt
   @param  void
   @return void.   */

void interrupts(){

    int i;
	
    unsigned int deviceStatus;
    
    /* Si assegna inizialmente la linea di INT_TIMER perchè le linee 0 e 1 non sono utilizzabili */
    int linea=INT_TIMER;
	
    /* Ricavo la linea che ha generato l' interrupt */
    while(linea<=INT_TERMINAL){
	if(CAUSE_IP_GET(((state_t *) INT_OLDAREA)->cause , linea)){ 
	    break;
	}
	linea++;			
    } 
    
    for(i=0; i<NUM_CPU; i++){
	
	/* controlliamo se si tratta di un interrupt o di una attesa del terminale */
	if(linea > INT_TERMINAL){ /* In questo caso nessun tipo di interrupt e' stato generato */ }
	else{
			
	    if (linea == INT_TIMER){/* in questo caso è scaduto il time slice*/
		
		/* controllo se il processo corrente è ancora in esecuzione */
		if(currProc[i] != NULL){ 
		    /* Stop e inserimento in ReadyQueue */
		    /* Setto lo state del currentThread con quello passato come parametro */
		    saveState(&(currProc[i]->p_s),(state_t *)INT_OLDAREA);
			
		    /* Aggiornamento cpu_time_used */
		    currProc[i]->time_cpu =+ (GET_TODLOW - INTIME[i]);
		}

		/* Altrimenti risetta da capo il time slice e chiama lo scheduler */		
		else{
		    SET_IT(SCHED_TIME_SLICE);
		    scheduler();
		}
	    }		
	    else{
		/* Chiamo il gestore dei device, passandogli la linea su cui si è verificato l'interrupt */
		deviceStatus=device(linea);
		
		/* Sblocco il processo in attesa di IO */		
		temp[i]=removeProcQ(&WaitIOList[i]);
		verhogen(temp[i]->p_semkey);
		
		/* Carico la OLD_AREA per far ripartire il codice*/
		LDST((state_t *)INT_OLDAREA);
		scheduler();
	    }
	}
	
    }

}

