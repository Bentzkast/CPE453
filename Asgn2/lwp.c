#include "lwp.h"
#include "scheduler.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef NULL
#define NULL 0
#endif

static tid_t tid = 0;
static scheduler currentSched = NULL;
thread list = NULL;
/* currently running process */
static thread running = NULL;
static rfile somewhere;
static thread exitnext = NULL;
static thread exitcur = NULL;

void addToList(thread new)
{
   thread current = list;
   if(list == NULL)
   {
      list = new;
      return;
   }
   
   while(current->lib_one != NULL)
   {
      current = current->lib_one;
   }
   current->lib_one = new;
   new->lib_two = current;
}

tid_t lwp_create(lwpfun fun,void * argument ,size_t stacksize)
{
   thread newthread = NULL;
   unsigned long *bp;
   unsigned long *sp;
   
   /* allocat:qed the thread and the stack */
   newthread = malloc(sizeof(struct threadinfo_st));
   if(!newthread)
   {
      perror(NULL);
      exit(1);
   }
   newthread->stack = malloc(stacksize * sizeof(unsigned long));
   if(!newthread->stack)
   {
      perror(NULL);
      free(newthread);
      exit(1);
   }
   newthread->stacksize = stacksize;
   tid++;
   newthread->tid = tid;
   /* initialize the thread register files*/
   newthread->state.rdi = (unsigned long) argument;
   newthread->state.fxsave=FPU_INIT;
   /* stack frame process */
   /* move the base pointer back into the high address*/
   sp = newthread->stack + stacksize;
   /* push the address of lwp_exit (return address)*/
   sp--;
   *sp = (unsigned long)lwp_exit;
   /* push the address of lwpfun */
   sp--;
   *sp = (unsigned long)fun;
   /* push crap & should never called(old base pointer ?)*/
   sp--;
   *sp = 0xAAAABBBB; /* garbage */
   bp = sp;   
   /* put the bp addrsss on the stck pointer pointing */
   newthread->state.rsp = (unsigned long)sp;
   newthread->state.rbp = (unsigned long)bp;

   if(currentSched == NULL)
   {
      currentSched = myRoundRobin;
   }
   currentSched->admit(newthread);
   addToList(newthread);
   return tid;
}

void lwp_exit(void)
{
   exitnext = NULL;
   exitcur = running;
   currentSched->remove(running);
   SetSP(somewhere.rsp);
   free(exitcur->stack);
   if((exitnext = currentSched->next()) == NULL)
   {
      lwp_stop();
   }
   else
   {
      running = exitnext;
      load_context(&(exitnext->state));
   }
}
tid_t lwp_gettid(void)
{
   if(running == NULL)
      return NO_THREAD;
   return running->tid;
}
void  lwp_yield(void)
{
   thread next = NULL;
   thread current = running;

   if((next = currentSched->next()) == NULL)
   {
      // What happen when yeild but no other
      lwp_stop();
   }
   else
   {
      running = next;
      if(current)
      {
         swap_rfiles(&(current->state),&(next->state));
      }
      else
      {
         load_context(&(next->state));
      }
   }
}
void  lwp_start(void)
{
   thread next = NULL;

   if(currentSched == NULL)
      currentSched = myRoundRobin;

   if((next = currentSched->next()) != NULL)
   {
      running = next;
      swap_rfiles(&somewhere,&(next->state));
   }
   else
   {
      save_context(&somewhere);
   }
}
void  lwp_stop(void)
{
   thread current = running;
   running = NULL;
   if(current)
      swap_rfiles(&(current->state),&somewhere);
   else
      load_context(&somewhere);
}

void removeList(thread victim)
{
   thread current = list;
   if(current == NULL || victim == NULL)
      return;
   if(current->tid == victim->tid)
   {
      list = victim->lib_one;
   }
   if(victim->lib_one)
      victim->lib_one->lib_two = victim->lib_two;
   if(victim->lib_two)
      victim->lib_two->lib_one = victim->lib_one;
}

void  lwp_set_scheduler(scheduler fun)
{
   thread temp = NULL;
   scheduler first = currentSched;
   if(first == NULL)
   {
      first = myRoundRobin;
   }
   if(fun == NULL)
   {
      fun = myRoundRobin;
   }
   
   if(fun->init)
   {
      fun->init();
   }
   while((temp = first->next()) != NULL)
   {
      first->remove(temp);
      fun->admit(temp);
   }
   if(first->shutdown)
   {
      first->shutdown();
   }
   currentSched = fun;
}
scheduler lwp_get_scheduler(void)
{
   return currentSched; 
}
thread tid2thread(tid_t tid)
{
   thread current = list;
   while(current && (current->tid != tid))
   {
      current = current->lib_one;
   }
   return current;
}
