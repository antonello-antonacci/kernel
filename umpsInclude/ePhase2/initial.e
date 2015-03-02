#ifndef INITIAL_E
#define INITIAL_E

#include "../const.h"
#include "../types11.h"
#include "../listx.h"
#include "../const11.h"
#include "../libumps.h"

extern void test ();
extern pcb_t *currProc[NUM_CPU];
extern struct list_head readyQueue[NUM_CPU];
extern struct list_head WaitIOList[NUM_CPU];
extern int chiavi[11];
extern int tickTimer[NUM_CPU];
extern U32 processCounter[NUM_CPU];
extern U32 soft_blockCounter[NUM_CPU];
extern cpu_t INTIME[NUM_CPU];
extern cpu_t timerS[NUM_CPU];
extern int pseudo_clock[NUM_CPU];
extern U32 todProc;
extern void popolaArea (memaddr area, memaddr gestore) ;

#endif
