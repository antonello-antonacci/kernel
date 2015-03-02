#ifndef SYSCALL_E
#define SYSCALL_E

#include "../const.h"
#include "../types11.h"
#include "../listx.h"


/* SysCall handling functions */

void saveState (state_t *current, state_t *old);
void sysBpHandler();
void prgtrap();
void tlbtrap();
int createProcess(state_t *statep, int prty);
int createBrother(state_t *statep, int prty);
void terminateProcess(pcb_t *cProc);
void verhogen(int semkey);
void passeren(int semkey);
cpu_t getCPUTime();
void waitClock();
unsigned int waitIO(int intNo, int dnum, int waitForTermRead);
void specPrgVec(state_t *oldArea, state_t *newArea);
void specTlbVec(state_t *oldp, state_t *newp);
void specSysVec(state_t *oldp, state_t *newp);


#endif
