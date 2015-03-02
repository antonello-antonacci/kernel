#ifndef SCHEDULER_E
#define SCHEDULER_E

#include "../const.h"
#include "../types11.h"
#include "../listx.h"
#include "../const11.h"

extern int isOn_nCPU;
pcb_t *temp[NUM_CPU];
void scheduler();

#endif
