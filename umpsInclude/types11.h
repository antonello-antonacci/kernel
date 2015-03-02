#ifndef _TYPES11_H
#define _TYPES11_H

#include <uMPStypes.h>
#include <listx.h>

// Process Control Block (PCB) data structure
typedef struct pcb_t {
	/*process queue fields */
	struct list_head	p_next;

	/*process tree fields */
	struct pcb_t		*p_parent;
	struct list_head	 p_child,
				 p_sib;
	

	/* processor state, etc */
	state_t       		p_s;     

	/* process priority */
	int					priority;
	
	/* key of the semaphore on which the process is eventually blocked */
	int					p_semkey;
	int 					isBlocked;
	
	U32 					time_cpu;
	
	/* nuove e vecchie aree per gestori */
	
	state_t 		*tlbNewArea;
	state_t 		*tlbOldArea;
	state_t 		*sysNewArea;
	state_t 		*sysOldArea;
	state_t 		*pgmNewArea;
	state_t 		*pgmOldArea;
	
	/* controllori di esecuzione delle sys 9 e 10 */
	int pgmMake;
	int tlbMake;

} pcb_t;



// Semaphore Descriptor (SEMD) data structure
typedef struct semd_t {
	struct list_head	s_next;
	
	// Semaphore value
	int					s_value;
	
	// Semaphore key
	int					s_key;
	
	// Queue of PCBs blocked on the semaphore
	struct list_head	s_procQ;
} semd_t;

struct {
	int disk[DEV_PER_INT];
	int tape[DEV_PER_INT];
	int network[DEV_PER_INT];  /*network? in const.h*/
	int printer[DEV_PER_INT];
	int terminalR[DEV_PER_INT];
	int terminalT[DEV_PER_INT];
} sem;


#endif
