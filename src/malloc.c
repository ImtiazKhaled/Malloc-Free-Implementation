// name: Imtiaz Mujtaba Khaled
// id: 1001551928

// name: Nahian Alam ( Group Leader )
// id:

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1)


static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void ){
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

typedef struct _block {
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *prev;  /* Pointer to the previous _block of allcated memory   */
   struct _block *next;  /* Pointer to the next _block of allcated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
}_block;


_block *freeList = NULL; /* Free list to track the _blocks available */

_block *next = NULL; /* Roving pointer for the Next Fit algorithm */

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 */
_block *findFreeBlock(_block **last, size_t size) 
{
   _block *curr = freeList;

#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size)) {
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0
   /* Best fit */
   _block *best = NULL;
   while ( curr ) {
      if( best == NULL ) {
         if( curr->free && curr->size >= size ){
            best = curr;
         }
      }
      else if( !(best == NULL) && (curr->free) && (curr->size < best->size) && (curr->size >= size) ){
         best = curr;
      }
      *last = curr;
      curr = curr->next;
   }
   curr = best;
#endif

#if defined WORST && WORST == 0
   /* Worst fit */
   _block *worst = NULL;
   while ( curr ) {
      if( worst == NULL ) {
         if( curr->free && curr->size >= size ){
            worst = curr;
         }
      }
      else if( !(worst == NULL) && (curr->free) && (curr->size > worst->size) ){
         worst = curr;
      }
      *last = curr;
      curr = curr->next;
   }
   curr = worst;
#endif

#if defined NEXT && NEXT == 0
   /* Next fit */
   if( next == NULL ) {
      while (curr && !(curr->free && curr->size >= size)) {
         *last = curr;
         curr  = curr->next;
      }
      if( curr == NULL ) {
         next = NULL;  
      } else {
         next = curr->next;
      }
   }else{
      curr = next;
      while (curr && !(curr->free && curr->size >= size)) {
         *last = curr;
         curr  = curr->next;
      }
      if( curr == NULL ) {
         next = NULL;  
      } else {
         next = curr->next;
      }
   }
#endif

   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
_block *growHeap(_block *last, size_t size) {
   /* Request more space from OS */
   ++num_grows;
   ++num_blocks;
   _block *curr = (_block *)sbrk(0);
   _block *prev = (_block *)sbrk(sizeof(_block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (_block *)-1) 
   {
      return NULL;
   }

   /* Update freeList if not set */
   if (freeList == NULL) 
   {
      freeList = curr;
   }

   /* Attach new _block to prev _block */
   if (last) 
   {
      last->next = curr;
   }

   max_heap += size;
   /* Update _block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) {
   
   num_requested += size;
   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block */
   _block *last = freeList;
   _block *next = findFreeBlock(&last, size);

   /* Split free _block if possible */
   if(next && next->size > size){
      ++num_splits;
      size_t split_size = next->size - size ;
      // Creates a new block, with the extra space from the allocated block
      _block *split_block = (_block *)sbrk(sizeof(_block) + split_size);
      // Sets values for the new block
      split_block->prev = next;
      split_block->next = next->next;
      split_block->size = split_size;
      split_block->free = true;
      next->next = split_block;
      ++num_blocks;
   }

   /* Could not find free _block, so grow heap */
   if (next == NULL) {
      next = growHeap(last, size);
   }else {
      ++num_reuses;
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) {
      return NULL;
   }else {
      ++num_mallocs;
   }
   
   /* Mark _block as in use */
   next->free = false;
 
   /* Return data address associated with _block */
   return BLOCK_DATA(next);
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) {
   if (ptr == NULL) {
      return;
   }

   /* Make _block as free */
   _block *curr = BLOCK_HEADER(ptr);
   
   assert(curr->free == 0);
   curr->free = true;
   ++num_frees;
   
   // Creates a struct for the next pointer
   _block *next_pointer = curr->next;
   // Coalesces if the next block is free
   while(curr && next_pointer && curr->free && next_pointer->free) {
      curr->size = next_pointer->size + curr->size + sizeof(_block);
      curr->next = next_pointer->next; 
      next_pointer = curr->next;
      ++num_coalesces;
      --num_blocks;
   }

   // Creates a struct for a previous pointer
   _block *prev_pointer = curr->prev;
   // Coalesces if the previous block is free
   while(curr && prev_pointer && curr->free && prev_pointer->free) {
      curr->size = prev_pointer->size + curr->size + sizeof(_block);
      curr->prev = prev_pointer->prev; 
      prev_pointer = curr->prev;
      ++num_coalesces;
      --num_blocks;
   }

}

/*
 * \brief calloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param nmemb number of elements the newly allocated memory is to be split into
 * \param size size of each of the elements in the requested memeory
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *calloc(size_t nmemb, size_t size){
   
   // Calculates the total size of the requested block
   size_t totalSize = nmemb * size;
   // Allocates a new block of memory with total requested size
   _block *res = malloc(totalSize);
   // Sets the pointer value of the new block to 0
   memset(BLOCK_DATA(res), 0, totalSize);
   return BLOCK_DATA(res);

}

/*
 * \brief realloc
 *
 * gets the pointer value to a previous block of memory, and re-allocates
 * it with more memory space
 * 
 * \param *ptr a pointer to the block of memory being expanded.
 * \param size size of the new requested memory in bytes
 * 
 * \return returns the requested memory re-allocation to the calling process 
 * or NULL if failed
 */
void *realloc(void *ptr, size_t size) {
   
   // Allocates a new block of memory with the new requested size
   _block *res = malloc(size);
   // Copies over the pointer value of the previous block to the new one
   memcpy(BLOCK_DATA(res), BLOCK_DATA(ptr), sizeof(*ptr));
   // Frees the previous pointer
   free(ptr);   
   return BLOCK_DATA(res);

}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
