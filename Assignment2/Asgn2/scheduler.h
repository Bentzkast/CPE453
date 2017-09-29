/* scheduler source file */
#ifndef SCHEDULER_H_
#define SCHEDULER_H_
#include <stdio.h>
#include <stdlib.h>
#include "lwp.h"


thread myList = NULL;
thread toExec = NULL;

void init(void);
void shutdown(void);
void my_admit(thread new);
void my_remove(thread victim);
thread my_next();

static struct scheduler rr_myscheduler = {NULL, NULL, my_admit, my_remove, my_next};
scheduler Robin = &rr_myscheduler;
/* add to the list */
void my_admit(thread new)
{
  thread temp = NULL;
  if(!myList)
  {
     myList = new;
  }
  else
  {
     temp = myList;
     while(temp->sched_one)
     {
        temp = temp->sched_one;
     }
     temp->sched_one = new;
     temp->sched_two = NULL;
     new->sched_one = NULL;
     new->sched_two = myList;
  }
}
/* remove from the list */
void my_remove(thread victim)
{
   return;
   thread temp = myList;
   if(!temp)
   {
      fprintf(stderr,"Trying to remove empty list");
      exit(1);
   }
   if(temp->tid == victim->tid)
   {
      if(temp->sched_one)
         temp->sched_one->sched_two = NULL;
      myList = temp->sched_one;
      return;
   }
   while(temp && temp->tid == victim->tid)
   {
      temp = temp->sched_one;
   }
   if(temp)
   {
      if(temp->sched_one)
      {
         temp->sched_one->sched_two = temp->sched_two;
      }
      if(temp->sched_two)
      {
         temp->sched_two->sched_one = temp->sched_one;
      }
   }
   else
   {
      fprintf(stderr,"remove awrong one");
      exit(1);
   }
}
/* return the thread in repating order */
thread my_next()
{
   thread temp = NULL;
   if(!toExec)
   {
      toExec = myList;
   }
   temp = toExec;
   if(toExec->sched_one == NULL)
   {
      toExec = myList;
   }
   toExec = toExec->sched_one;
   return temp;
}
#endif
