/* Il seguente File gestisce la lista dei PCB e i PCB_Tree */

/* GRUPPO: Alessandro Calo' - Alberto Scarlino - Manuela Corina - Antonello Antonacci*/

#include "../umpsInclude/const11.h" 
#include "../umpsInclude/listx.h" 
#include "../umpsInclude/types11.h"

#include "../umpsInclude/ePhase1/pcb.e"
#include "../umpsInclude/ePhase1/asl.e"


/* Dichiarazione variabili globali di Gestione dei PCB */

/* Vettore dei Pcb Liberi */
HIDDEN pcb_t pcbFree_table[MAXPROC];

/* Elemento Sentinella del Vettore */
HIDDEN struct list_head pcbFree_h;

/* Indici dei Pcb con priority massima e minima */
int maxPrt;
int lowPrt;


/* FUNZIONI DI GESTIONE DELLA CODA DEI PCB */


/* Inserisce un elemento nella lista dei Pcb liberi
      @param p: puntatore al pcb da inserire
      @return:  nessun parametro*/
void freePcb(pcb_t *p){
    list_add(&p->p_next, &pcbFree_h);
}


/*Alloca un Pcb rimuovendolo dalla lista dei Pcb Liberi
      @param: nessun parametro
      @return: NULL se la lista dei pcb è vuota altrimenti restituisce un puntatore al pcb allocato */
pcb_t *allocPcb(void){

    pcb_t *tempPcb;
    int i;
    
    if (list_empty(&pcbFree_h)) return NULL;
    else{
	tempPcb = container_of(pcbFree_h.next, pcb_t, p_next);
	list_del(pcbFree_h.next);

	/* Inizializzazione delle liste e dei parametri di stato */
	INIT_LIST_HEAD(&tempPcb->p_child);
	INIT_LIST_HEAD(&tempPcb->p_sib);
	
	tempPcb->p_s.entry_hi = 0;
	tempPcb->p_s.cause = 0;
	tempPcb->p_s.status = 0;
	tempPcb->p_s.pc_epc = 0;
	tempPcb->p_s.hi = 0;
	tempPcb->p_s.lo = 0;

	tempPcb->priority = 0;
	tempPcb->isBlocked = 0;
	
	for (i = 0; i < 29; i++) {
	    tempPcb->p_s.gpr[i] = 0;
	}
	
	return(tempPcb);
    }
}


/* Inizializza le strutture di gestione dei Pcb 
    @param: nessun parametro
    @return: nessun parametro */
void initPcbs(void){

    pcb_t *pcb;
    int i;

    INIT_LIST_HEAD(&pcbFree_h);

    for (i=0; i < MAXPROC; i++){
	pcb = &pcbFree_table[i];
	freePcb(pcb);
    }
}


/* Inizializza una lista di pcb vuota */
/* @param head: puntatore alla lista dei pcb vuota 
   @return: nessun parametro */
void mkEmptyProcQ(struct list_head *head){
      
    INIT_LIST_HEAD(head);
}


/* Controlla se la lista dei pcb è vuota */
/* @param head: puntatore alla lista dei pcb
   @return: TRUE (1) se la lista è vuota, altrimenti FALSE(0) */
int emptyProcQ(struct list_head *head){

    if (list_empty(head)) return (1);
    else return (0);
}


/* Inserisce un pcb in nella lista dei pcb in base alla sua priorita (in ordine decrescente) */
/* @param head: puntatore alla lista dei pcb
   @param p: puntatore al pcb da inserire
   @return: nessun parametro */
void insertProcQ(struct list_head *head, pcb_t *p){

    pcb_t *tempPcb;
    pcb_t *posPcb;
    
    /* Se la lista è vuota inserisce il primo elemento e setta la sua priority come unica */
    if (emptyProcQ(head)) {
	list_add_tail(&p->p_next, head);
	maxPrt = p->priority;
	lowPrt = p->priority;
    }
    else {
	/* Altrimenti: se la priority e minore di quella dell'elemento minore della lista inseriamo p in coda */
	if (p->priority < lowPrt) {
	  list_add_tail(&p->p_next, head);
	  lowPrt = p->priority;
	}
	else { 
	   /* Se la priority e maggiore di quella dell'elemento maggiore della lista inseriamo p in testa */
	   if (p->priority >= maxPrt) {
	     list_add(&p->p_next, head);
	     maxPrt = p->priority;
	   }
	   else { 
	     /* Altrimenti cerchiamo nella lista la posizione dove inserire p */
	     list_for_each_entry(tempPcb, head, p_next){
		if (tempPcb->priority >= p->priority) posPcb = tempPcb;
	      }
	      /* E lo inseriamo in coda all'ultimo elemento con priority maggiore o uguale a quella di p */
	      list_add_tail (&p->p_next, &posPcb->p_next);
    		
	   }
    		
    	}
    }
    
}


/* Rimuove il primo pcb dalla lista dei pcb */
/* @param head: puntatore alla lista dei pcb
   @return NULL se la lista è vuota, altrimenti restituisce il puntatore al pcb rimosso */
pcb_t *removeProcQ(struct list_head *head){
	
    pcb_t *remPcb;

    /* Restituisce NULL se la coda è vuota */
    if (list_empty(head)) return NULL;
    else {
      
	/* Altrimenti preleva il pcb dalla coda, lo rimuove e lo restituisce */
	remPcb = container_of(head->next, pcb_t, p_next);
	list_del(head->next);

	return (remPcb);
    }

}


/* Rimuove il pcb specificato dalla lista dei pcb */
/* @param head: puntatore alla lista dei pcb
   @param p: puntatore al pcb da rimuovere
   @return NULL se il pcb non è nella lista, altrimenti ritorna il puntatore al pcb rimosso */
pcb_t *outProcQ(struct list_head *head, pcb_t *p){

    pcb_t *newPcb;

    if (list_empty(head)) return NULL;
    else {
	/* Se la coda non è vuota, cerca p nella coda dei processi*/
	list_for_each_entry(newPcb, head, p_next){
	    /* Se lo trova, lo elimina dalla coda e lo restituisce */
	    if (p == newPcb){
		list_del(&p->p_next);
		return (p);
	    }
	}

	/* Se non lo trova restituisce NULL*/
	return NULL;
    }
}


/* Permette di avere la testa della lista dei pcb */
/* @param head: puntatore alla lista dei pcb
   @return: NULL se la lista dei pcb è vuota, altrimenti restituisce un puntatore al primo elemento della lista dei pcb */
pcb_t *headProcQ(struct list_head *head){

    pcb_t *headPcb;

    /* Se la coda è vuota ritorna NULL */
    if (list_empty(head)) return NULL;
    else {
	/*Altrimenti ritorna il pcb*/
	headPcb = container_of(head->next, pcb_t, p_next); 
	return (headPcb);
    }
}



/* TREE VIEW FUNCTIONS */


/* Controlla se il pcb non ha figli */
/* @param this: puntatore al pcb
   @return: TRUE (1) se pcb non ha figli, FALSE(0) altrimenti */
int emptyChild(pcb_t *this){

    if (list_empty(&this->p_child)) return (1);
    else return (0);
}


/* Assegna a un pcb genitore un pcb figlio inserendolo in coda alla lista dei figli */
/* @param prnt: puntatore al pcb genitore
   @param p: puntatore al pcb figlio
   @return: nessun parametro */
void insertChild(pcb_t *prnt, pcb_t *p){
 
    list_add_tail(&p->p_sib, &prnt->p_child);
    p->p_parent = prnt;
}


/* Rimuove il primo figlio di un pcb genitore specificato */
/* @param p: puntatore al pcb genitore
   @return: NULL se il pcb genitore non ha figli, altrimenti restituisce il puntatore al pcb figlio */
pcb_t *removeChild(pcb_t *p){

    pcb_t *child;

    if (list_empty(&(p->p_child))) return NULL;
    else {
	/* prende il primo figlio */
    	child = container_of(p->p_child.next, pcb_t, p_sib);
	
	/* restituisce il figlio rimosso e elimina il puntatore al padre */
	list_del(&(child->p_sib));
	child->p_parent = NULL;
	
	return child;
    }
}


/* Rimuove dalla lista un figlio specificato */
/* @param p: puntatore al pcb figlio
   @return: NULL se il pcb non ha un genitore, altrimenti restituisce il puntatore al pcb figlio rimosso */
pcb_t *outChild(pcb_t *p){
      
    pcb_t *parent;
   
    if (&p->p_parent == NULL) return NULL;
    else {
	/* prende il figlio del genitore da rimuovere */
	parent=container_of(&p->p_parent->p_sib, pcb_t, p_sib); 

	/* restituisce il figlio rimosso e elimina il puntatore al padre */
	list_del(&(p->p_sib));
	p->p_parent = NULL;

	return (p);	
    }
} 
