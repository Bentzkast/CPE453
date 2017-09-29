#include "scheduler.h"
#include <stdio.h>
thread myList = NULL;
thread toExec = NULL;

static struct scheduler rr_mypublish = { NULL, 
   NULL, my_admit, my_remove, my_next};
scheduler myRoundRobin = &rr_mypublish;
void my_init(void){}
void my_shutdown(void){}

void my_admit(thread new)
{
   thread temp = myList;
   if(myList == NULL)
   {
      myList = new;
      myList->sched_one = NULL;
      myList->sched_two = NULL;
   }
   else
   {
      while(temp->sched_one != NULL)
      {
         temp = temp->sched_one;
      }
      new->sched_one = NULL;
      new->sched_two = temp;
      temp->sched_one = new;
   }
}

void my_remove(thread victim)
{
   thread temp = myList;
   if(temp == NULL || victim == NULL)
      return;
   if(temp->tid == victim->tid)
   {
      myList = victim->sched_one;
   }
   if(victim->sched_one)
      victim->sched_one->sched_two = victim->sched_two;
   if(victim->sched_two)
      victim->sched_two->sched_one = victim->sched_one;
}

thread my_next()
{  
   if(toExec == NULL)
   {
      toExec = myList;
   }
   else
   {
      toExec = toExec->sched_one;
      if(toExec == NULL)
         toExec = myList;
   }
   return toExec;
}
