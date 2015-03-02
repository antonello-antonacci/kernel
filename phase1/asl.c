/* Il seguente File gestisce la coda dei processi bloccati attraveso l'uso dei semafori */

/* GRUPPO: Alessandro Calo' - Alberto Scarlino - Manuela Corina - Antonello Antonacci */

#include "../umpsInclude/const11.h" 
#include "../umpsInclude/listx.h" 
#include "../umpsInclude/types11.h"

#include "../umpsInclude/ePhase1/pcb.e"
#include "../umpsInclude/ePhase1/asl.e"


/* Dichiarazione variabili globali di Gestione dei Thread */

/* Vettore dei Semafori Liberi */
HIDDEN semd_t semd_table[MAXPROC];

/* Elemento Sentinella del Vettore dei Semafori liberi */
HIDDEN struct list_head semdFree_h;

/* Elemento Sentinella del Vettore dei Semafori bloccati */
HIDDEN struct list_head semd_h;




/*Alloca un semaforo rimuovendolo dalla lista dei semdFree_h
	@param: head testa della coda dei descrittori 
	@return: NULL se la lista dei descrittori è vuota altrimenti restituisce un puntatore al semaforo allocato */
semd_t *allocSemd(struct list_head *head){

   semd_t *tempSemd;

   if (list_empty(head)) return NULL;
   else 
      tempSemd = container_of(head->next, semd_t, s_next);
      list_del(head->next);
    
   return (tempSemd);
}


/* Inizializza  la lista dei descrittori di semafori contenente tutti gli elementi della semd_table
	@param: nessun parametro
	@return: nessun parametro */	
void initASL(){

   semd_t *semd;
   
   int i = 0;

   INIT_LIST_HEAD(&semdFree_h);
   INIT_LIST_HEAD(&semd_h);

   for (i=0; i < MAXPROC; i++){
	semd = &semd_table[i];
	list_add(&semd->s_next, &semdFree_h);
    }
}


/* Restituisce un puntare al semaforo con chiave pari a key 
	@param: key descrittore di semaforo
	@return: NULL se non esiste un elemento con chiave Key altrimenti restituisce il puntatore semd */	
semd_t*	getSemd(int key){

    semd_t *new_semd;

    if (list_empty(&semd_h)) return NULL;
    else {
	/* Se la coda non e' vuota, cerca p nella coda dei processi*/
	list_for_each_entry(new_semd, &semd_h, s_next){
	    /* Se lo trova, lo restituisce */
	    if (key == new_semd->s_key){
		return (new_semd);
	    }
	}

	/* Se non lo trova restituisce NULL*/
	return NULL;
    }
}


/*Inserisce il  PCB nella coda dei processi bloccati al semaforo con chiave key
	@param: key descrittore del semaforo
	@param: p puntatore al pcb da inserire nella coda di pcb .
	@return: NULL se la coda dei semafori è vuota. */
int insertBlocked(int key, pcb_t *p){

    semd_t *new_semd;

    /* Cerca il semaforo con chiave key nella lista dei semafori attivi */
    list_for_each_entry(new_semd, &semd_h, s_next){
	/* Se lo trova, inserisce p e ritorna FALSE (0)*/
	if (new_semd->s_key == key){
	  p->p_semkey = key;
	  p->isBlocked = 1;
	  insertProcQ(&new_semd->s_procQ, p);
	  return FALSE;
	}
    }

    /* Se non lo trova, cerca un semd nella coda di quelli liberi */
    new_semd = allocSemd(&semdFree_h);
    /* Se non ci sono semd liberi ritorna TRUE (1) */
    if (new_semd == NULL) return TRUE;
    /* Altrimenti lo elimina dalla coda dei semd liberi, setta key e s_procQ e lo inserisce nella coda dei semd utilizzati. Ritorna FALSE (0)*/
    else {
	new_semd->s_key = key;
	INIT_LIST_HEAD(&new_semd->s_procQ);
	p->p_semkey = key;
	p->isBlocked = 1;
	insertProcQ(&new_semd->s_procQ, p);
	list_add(&new_semd->s_next, &semd_h);
	return FALSE;
    }
}


/* Ritorna il primo pcb dalla coda dei processi bloccati
	@param: key descrittore di semaforo 
	@return: NULL se il descrittore non esiste nella coda di pcb, altrimenti restituisce il pcb rimosso. */

pcb_t* removeBlocked(int key){

   pcb_t *tempPcb;
   semd_t *tempSemd;
   
   /* Cerchiamo il descrittore con chiave key */
   tempSemd = getSemd(key);
   /* se non esiste ritorniamo NULL */
   if (tempSemd == NULL) return NULL;
   /* Altrimenti rimuoviamo e ritorniamo il primo pcb della lista del semd */
   else {
   	tempPcb = removeProcQ(&tempSemd->s_procQ);
	tempPcb->isBlocked = 0;
	/* se la coda s_procQ del semaforo resta vuota dopo l'eliminazione del pcb, elimina anche il semaforo corrispondente da quelli utilizzati
	   e lo restituisce alla vettore di quelli liberi */
	if (list_empty(&tempSemd->s_procQ)){
		list_del(&tempSemd->s_next);
		list_add(&tempSemd->s_next, &semdFree_h);
	}

	return (tempPcb);
   }
}


/* Rimuove il Pcb dalla coda del semaforo 
	@ param: p puntatore al Pcb da rimuovere
	@return: NULL se il Pcb non è presente nella coda. Altrimenti restiruisce il puntatore al Pcb rimosso */	
pcb_t* outBlocked(pcb_t *p){

   pcb_t *remPcb;
   semd_t *remSemd;

   /* Cerchiamo il semaforo su cui e' bloccato p */
   remSemd = getSemd(p->p_semkey);
   /* se non c'e' restituiamo NULL */
   if (remSemd == NULL) return (NULL);
   /* Altrimenti eliminiamo p dalla coda di tale semaforo */
   remPcb = outProcQ(&remSemd->s_procQ, p);
   remPcb->isBlocked = 0;
   /* Anche qui se s_procQ resta NULL eliminiamo il semd corrispondente e lo restituiamo a quelli liberi */
   if (list_empty(&remSemd->s_procQ)){
		list_del(&remSemd->s_next);
		list_add(&remSemd->s_next, &semdFree_h);
		return (remPcb);
	}
   else return (remPcb);
}



/* Restituisce il puntatore al Pcb che si trova in testa alla coda  dei Pcb
	@param: key descrittore  del semaforo
	@return: NULL se il descrittore non esiste nella coda dei processi, oppure se la coda dei processi è vuota. 
		      Altrimenti  restituisce la testa della lista dei Pcb */			  
pcb_t* headBlocked(int key){

   pcb_t *headPcb;
   semd_t *tempSemd;

   /* Cerchiamo il descrittore del semaforo */
   tempSemd = getSemd(key);
   /* Se non c'e' ritorniamo NULL */
   if (tempSemd == NULL) return NULL;
   else {
	/* Altrimenti prendiamo la testa della lista dei pcb e la restituiamo */
	headPcb = headProcQ(&tempSemd->s_procQ);
	/* Se la lista fosse vuota, restituiamo NULL */
	if (headPcb == NULL) return NULL;
	else return (headPcb);
   }
}



/* Rimuove dalla coda dei Pcb un figlio specificato
	@param: p puntatore al Pcb da rimuovere
	@return: nessun parametro */	
void outChildBlocked(pcb_t *p){

   pcb_t *remPcb;
   pcb_t *tempPcb;

   /* Eliminiamo il pcb indicato da p */
   remPcb = outBlocked(p);
   /* se p ha anche dei figli */
   if (!(list_empty (&remPcb->p_child))){
      list_for_each_entry(tempPcb, &remPcb->p_child, p_sib){
	  /* ripetiamo la outChildBlocked sulla lista dei suoi figli */
	  outChildBlocked (tempPcb);
      }
   }
}
