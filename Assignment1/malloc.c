#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define BASE_SIZE 64000

typedef struct Chunk
{
   size_t size;
   int free;
   struct Chunk* next;
   intptr_t spaceEnd;
}Header;

Header* head = NULL;

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
   return calloc;
}

void free(void *ptr)
{
   Header *toBeFree;
   if(!ptr)
   {
      return;
   }
   toBeFree = (Header*)ptr - 1;

   toBeFree->free = 1; 
}

void *realloc(void *ptr, size_t size)
{
   Header *toBeRe;
   void * new;

   if(ptr == NULL)
   {
      return malloc(size);
   }
      
   toBeRe = (Header*)ptr - 1;
   if(toBeRe->size >= size)
   {
      return (split(toBeRe,size) + 1);
   }
   new = malloc(size);
   if(new == NULL)
   { 
      return NULL;
   }
   memcpy(new, ptr, toBeRe->size);
   free(ptr);
   return new;
}
