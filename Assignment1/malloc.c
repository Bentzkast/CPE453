#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define DEBUG_MALLOC
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
   return (void*)(current + 1);
}

void* calloc(size_t nmemb, size_t size)
{
   size_t totalSize = nmemb * size;
   void * tmp = malloc(totalSize);
   memset(tmp, 0, totalSize);
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
}

void *realloc(void *ptr, size_t size)
{
   Header *toBeRe;
   Header *last;
   void * new;

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
   /* smaller Chunk */
   if(toBeRe->size >= size)
   {
      new = (void*)(split(toBeRe,size) + 1);

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
   return new;
}
