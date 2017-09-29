#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define BASE_SIZE 64000

typedef struct Chunk
{
   size_t size;
   int free;
   struct Chunk* next;
   intptr_t spaceEnd;
}Header;

Header* head = NULL;

/* find any mergeable chunk */
void merge()
{
   Header *temp = head;
   while(temp && temp->next)
   {
      if(temp->free && temp->next->free)
      {
         temp->size += temp->next->size + sizeof(Header);
         temp->spaceEnd = temp->next->spaceEnd;
         temp->next = temp->next->next;
         temp->free = 1;
      }
      else
      {
         temp = temp->next;
      }
   }
}

/* split the chunks into two with it own header
 * the inputed dont need to take acount of old header 
 * the caller need to check before hand*/
Header* split(Header* current,size_t needed)
{
   /* if split not possible, data free is to small for a new header 
    * do nothing */
   if(current->size <= needed + sizeof(Header))
   {
      return current;
   }
   else
   {
      /* new header and its free space */
      Header* temp = (Header*)((intptr_t)(current + 1) + needed);
      temp->spaceEnd = current->spaceEnd;
      temp->size = current->size - needed - sizeof(Header);
      temp->free =1;
      temp->next = current->next;

      /* the now smaller chunk */
      current->next = temp;
      current->free = 0;
      current->size = needed;
      current->spaceEnd = (intptr_t)(current+1) + (needed - 1) ;
      return current;
   }
}

/* prev is the last in the list
 * size Requested need to be divisible by 16
 * */
Header* allocateSpace(Header* last,size_t sizeReq)
{
   Header* temp;
   size_t totalSize = sizeReq + sizeof(Header);
   size_t sizeAllocated;
   
   (totalSize > BASE_SIZE) ? (sizeAllocated = totalSize) : 
      (sizeAllocated = BASE_SIZE);

   temp = sbrk(sizeAllocated);
   /* error sbrk */
   if(temp == (void*) -1 )
   {
      return NULL;
   }
   /* create a free header first */
   if(last != NULL)
   {
      last->next = temp; 
   }
   temp->size = sizeAllocated - sizeof(Header);;
   temp->free = 1;
   temp->next = NULL;
   temp->spaceEnd = (intptr_t)(temp + 1) + temp->size;
   
   if(temp->size == sizeReq)
   {
      temp->free = 0;
   }
   else
   {
      split(temp,sizeReq);
   }
   return temp;
}
/* try to find a legible free space
 * return NULL if there aren't any */
Header* searchFree(size_t sizeNeeded)
{
   Header *temp = head; 
   size_t totalSize = sizeNeeded + sizeof(Header);
   while(temp && !(temp->free && temp->size >= totalSize))
   {
      temp = temp->next;
   }
   return temp;
}

void* malloc(size_t size)
{
   Header* current;
   int originalSize = size;
   char buffer[100];
   int print = 0;
   
   if(size <=0)
   {
      return NULL;
   }
   /* allign the size to modable by 16 */
   while(size % 16 != 0)
   {
      size++;
   }
   /* case empty chunk, first time malloc */
   if(head == NULL)
   {
      current = allocateSpace(NULL, size);
      if(current == NULL)
      {
         return NULL;
      }
      head = current;
   }
   /* case already there */
   else
   {
      Header* available;
      available = searchFree(size);
      /* if there enough space in the list */
      if(available != NULL)
      {
         current = split(available,size);
      }
      else
      {
         /* new to create new chunk */
         Header* last = head;
         while(last->next != NULL)
         {
            last = last->next;
         }
         current = allocateSpace(last,size);
         if(current == NULL)
         {
            return NULL;
         }
      }
   }
   /* debug malloc env variable */
   if(getenv("DEBUG_MALLOC"))
   {
      print = snprintf(buffer,100,"MALLOC: malloc(%d) => (ptr=%p, size%d)\n"
         ,originalSize,(void*)(current+1),(int)size);
      if(write(STDERR_FILENO, buffer, print) == -1)
      {
         perror(NULL);
         exit(EXIT_FAILURE);
      }
   }
   return (void*)(current + 1);
}

void* calloc(size_t nmemb, size_t size)
{
   /* not sure if this could wrap around or not */
   size_t totalSize = nmemb * size;
   void * tmp = malloc(totalSize);
   memset(tmp, 0, totalSize);
   /* debug malloc env variable */
   if(getenv("DEBUG_MALLOC"))
   {
      char buffer[100];
      int print = 0;
      print = snprintf(buffer,100,"MALLOC: calloc(%d,%d) => (ptr=%p, size%d)\n"
         ,(int)nmemb,(int)size,tmp,(int)(((Header*)tmp -1)->size));
      if(write(STDERR_FILENO, buffer, print) == -1)
      {
         perror(NULL);
         exit(EXIT_FAILURE);
      }
   }
   return tmp;
}

Header* findBlock(Header **last,void *ptr)
{
   Header *temp = head;
   intptr_t adr = (intptr_t)ptr;
   while(temp && (adr > temp->spaceEnd))
   {
      *last = temp;
      temp = temp->next;
   }
   return temp;
}

void free(void *ptr)
{
   Header *toBeFree;
   Header *last;
   if(!ptr)
   {
      return;
   }
   toBeFree = findBlock(&last,ptr);
   toBeFree->free = 1; 
   merge(); 
   /* debug malloc env variable */
   if(getenv("DEBUG_MALLOC"))
   {
      char buffer[100];
      int print = 0;
      print = snprintf(buffer,100,"MALLOC: free(%p)\n"
         ,toBeFree+1);
      if(write(STDERR_FILENO, buffer, print) == -1)
      {
         perror(NULL);
         exit(EXIT_FAILURE);
      }
   }
}

void debugRealloc(void *oldPtr,int sizeReq,void* newPtr,int newSize)
{
   if(getenv("DEBUG_MALLOC"))
   {
      char buffer[100];
      int print = 0;
      print = snprintf(buffer,100,"MALLOC: realloc(%p,%d) => (ptr=%p,size=%d)\n"
         ,oldPtr,sizeReq,newPtr,newSize);
      if(write(STDERR_FILENO, buffer, print) == -1)
      {
         perror(NULL);
         exit(EXIT_FAILURE);
      }
   }
}


void *realloc(void *ptr, size_t size)
{
   Header *toBeRe;
   Header *last;
   void * new;
   int originSize = size;

   if(ptr == NULL)
   {
      return malloc(size);
   }
   if(size == 0)
   {
      free(ptr);
      return NULL;
   }
   toBeRe = findBlock(&last,ptr);
   if(toBeRe == NULL)
   {
      return NULL;
   }
   /* allign the size to modable by 16 */
   while(size % 16 != 0)
   {
      size++;
   }
   /* smaller Chunk */
   if(toBeRe->size >= size)
   {
      new = (void*)(split(toBeRe,size) + 1);
      debugRealloc(ptr,originSize,new,size);
      return new;
   }
   /* if we can eat nearby chunk */
   else
   {  
      if(toBeRe->next && toBeRe->next->free)
      {
         toBeRe->size += toBeRe->next->size + sizeof(Header);
         toBeRe->spaceEnd = toBeRe->next->spaceEnd;
         toBeRe->next = toBeRe->next->next;
         toBeRe->free = 0;
      }
      if(toBeRe->size >= size)
      {
         new = (void*)(split(toBeRe,size) + 1);
         debugRealloc(ptr,originSize,new,size);
         return new;
      }
   }
   /* forced to make new colony */
   new = malloc(size);
   if(new == NULL)
   { 
      return NULL;
   }
   memcpy(new, ptr, toBeRe->size);
   free(ptr);
   debugRealloc(ptr,originSize,new,size);
   return new;
}
