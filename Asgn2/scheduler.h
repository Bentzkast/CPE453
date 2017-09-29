#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "lwp.h"
#ifndef NULL
#define NULL 0
#endif

void my_init(void);
void my_shutdown(void);
void my_admit(thread new);
void my_remove(thread victim);
thread my_next();

thread myList;
thread toExec;
scheduler myRoundRobin;

#endif
