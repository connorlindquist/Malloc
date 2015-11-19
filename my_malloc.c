//Connor Lindquist
//902990968
#include "my_malloc.h"

/* You *MUST* use this macro when calling my_sbrk to allocate the 
 * appropriate size. Failure to do so may result in an incorrect
 * grading!
 */
#define SBRK_SIZE 2048

/* If you want to use debugging printouts, it is HIGHLY recommended
 * to use this macro or something similar. If you produce output from
 * your code then you will receive a 20 point deduction. You have been
 * warned.
 */
#ifdef DEBUG
#define DEBUG_PRINT(x) printf x
#else
#define DEBUG_PRINT(x)
#endif


/* make sure this always points to the beginning of your current
 * heap space! if it does not, then the grader will not be able
 * to run correctly and you will receive 0 credit. remember that
 * only the _first_ call to my_malloc() returns the beginning of
 * the heap. sequential calls will return a pointer to the newly
 * added space!
 * Technically this should be declared static because we do not
 * want any program outside of this file to be able to access it
 * however, DO NOT CHANGE the way this variable is declared or
 * it will break the autograder.
 */
void* heap;

/* our freelist structure - this is where the current freelist of
 * blocks will be maintained. failure to maintain the list inside
 * of this structure will result in no credit, as the grader will
 * expect it to be maintained here.
 * Technically this should be declared static for the same reasons
 * as above, but DO NOT CHANGE the way this structure is declared
 * or it will break the autograder.
 */
metadata_t* freelist[8];
/**** SIZES FOR THE FREE LIST ****
 * freelist[0] -> 16
 * freelist[1] -> 32
 * freelist[2] -> 64
 * freelist[3] -> 128
 * freelist[4] -> 256
 * freelist[5] -> 512
 * freelist[6] -> 1024
 * freelist[7] -> 2048
 */

static int mylog2(double val);
static void addToHead(metadata_t* block);
static metadata_t* removeHead(int index);
static metadata_t* blockRemove(metadata_t* block);
int memSplit(int start, int end);
static metadata_t* buddyFinder(metadata_t *block);
static int buddyMerge(metadata_t *block);
void print_freelist();

/* this function should allocate a block that is big enough to hold the
 * specified size, and that is all. if there is not a block that is able
 * to satisfy the request, then you should attempt to grab more heap
 * space with a call to my_sbrk. if this succeeds, then you should continue
 * as normal. If it fails (by returning NULL), then you should return NULL.
 */
void* my_malloc(size_t size)
{
	static int once = 1; //Replaces boolean logic to do heap setup once
  	if(size + sizeof(metadata_t) > SBRK_SIZE) {
  		ERRNO = SINGLE_REQUEST_TOO_LARGE; //Cannot allocate more than SBRK_SIZE block
  		return NULL;
 	 }
 	 if(size <= 0) {
 	 	return NULL;
 	 }
 	 if(once) {
 	 	heap = my_sbrk(SBRK_SIZE);
 	 	if(heap == NULL) {
 	 		return NULL; //Ensure heap can be initialized
 	 	}
	 	once = 0;
	 	freelist[7] = heap; //Heap setup
		freelist[7]->in_use = 0;
		freelist[7]->next = freelist[7]->prev = NULL;
		freelist[7]->size = SBRK_SIZE;
 	 }

 	 double blockSize = size + sizeof(metadata_t); //Keep track of size of block needed
 	 int index = mylog2(blockSize/16);
 	 if(freelist[index] == NULL) {
 	 	int split = 0;
 	 	int larger;
 	 	for(larger = index + 1; larger < 8; larger++) {
 	 		if(freelist[index] != NULL) {
 	 			split = memSplit(larger, index); //Split block at larger
 	 			break;
 	 		}
 	 	}
 	 	if(!split) {
 	 		void *newHeap = my_sbrk(size); //Create new heap to allocate more space
 	 		if(newHeap == NULL) {
 	 			ERRNO = OUT_OF_MEMORY; //Checks for chance to run out of memory
 	 			return NULL;
 	 		}
 	 		freelist[7] = newHeap; //Initalize new heap
			freelist[7]->in_use = 0;
			freelist[7]->next = freelist[7]->prev = NULL;
			freelist[7]->size = SBRK_SIZE;
			memSplit(7, index);
 	 	}
 	 }
 	 metadata_t *block = removeHead(index);
 	 block->in_use = 1; //Set in use, etc
 	 void *res = (void *)(block + 1);
 	 ERRNO = NO_ERROR; //Ensure ERRNO is set to 0
 	 return res;
}

/* this function returns a pointer to an array of num quantity of elements
 * each of size size*/
void* my_calloc(size_t num, size_t size)
{
	//print_freelist();
	if(num * size > SBRK_SIZE) {
		ERRNO = SINGLE_REQUEST_TOO_LARGE; //Check for error in requesting too much memory
		return NULL;
	}
	void* ammt = my_malloc(num * size);
	int i;
	for (i = 0; i < (((metadata_t*)ammt - 1)->size - sizeof(metadata_t)); i++)
	{
		*((char *) ammt + i) = 0;
	}
	ERRNO = NO_ERROR; //Ensure ERRNO is set to 0
	return ammt;
}

/* this function should free the block of memory, recursively merging
 * buddies up the freelist until they can be merged no more.
 */
void my_free(void* ptr)
{
	if(ptr == NULL) {
		return; //Cannot free NULL
	}
	if((char *)ptr < (char *)heap - sizeof(metadata_t)) {
		return; //
	}
	metadata_t *tbFree = (metadata_t *)ptr - 1; //To be freed memory
	if(!tbFree->in_use) {
		return; //Return if not in_use / no need to free
	}
	tbFree->in_use = 0; //Update in_use
	buddyMerge(tbFree); //External method to merge
}

/* this copies memory starting at the source, src and copying num_bytes
 * of bytes into the memory pointed to by dest. The source and destination
 * can indeed overlap, so the copy should be done as if the data were put into
 * a temporary buffer first and then copied. */
void* my_memmove(void* dest, const void* src, size_t num_bytes)
{
  	if(dest == src) {
  		ERRNO = NO_ERROR;
  		return dest;
  	}
  	int marker = 0;
  	char *des = (char *)dest;
  	char *sor = (char *)src;
  	if(des > sor) {
  		marker = num_bytes - 1;
  		des[marker] = sor[marker]; //Set in des and sor
  		marker--;
  	} else {
  		while(marker < num_bytes) {
  			des[marker] = sor[marker]; //Set in des and sor while marker < num_bytes
  			marker++;
  		}
  	}
  	ERRNO = NO_ERROR; //Ensure ERRNO is set
  	return dest;
}

/*
 * Log function instead of using log2 from math.h
 */
static int mylog2(double val) {
	int ans = 0;
	int factor = 1; //Acts as a multiple of two
	while(factor < val) {
		ans++;
		factor = factor * 2; //Multiple of two
	}
	return ans;
}

/*
 * Simple add to head method
 */
static void addToHead(metadata_t* block) {
	int index = mylog2((block->size)/16);
	if(freelist[index] != NULL) {
		block->next = freelist[index];
		freelist[index]->prev = block;
	}
	freelist[index] = block;
}

/*
 * Simple remove head of list
 * Returns head
 */
static metadata_t* removeHead(int index) {
	metadata_t *head = freelist[index];
	freelist[index] = head->next;
	if(freelist[index] != NULL) {
		freelist[index]->prev = NULL;
	}
	head->next = NULL;
	return head;
}

/*
 * External method to remove block
 */
static metadata_t* blockRemove(metadata_t* block) {
	if(block->prev == NULL) { //Block is head or stand-alone block
		int index = mylog2((block->size)/16);
		if(freelist[index] == block) { //Check if block is head of list
			freelist[index] = block->next;
			if(block->next != NULL) {
				block->next->prev = NULL;
			}
			block->next = NULL;
		}
	} else { //Block isn't head
		block->prev = block-> next = NULL; //Set pointers to remove
		block->prev->next = block->next;
		block->next->prev = block->prev;
	}
	return block;
}

/*
 * Buddy Finder method to find block to pair up with
 */
static metadata_t* buddyFinder(metadata_t *block) {
	uintptr_t shift = (uintptr_t)(mylog2((double)block->size));
	metadata_t* buddy = (metadata_t *)((((long)block - (long)heap) ^ (1 << shift)) + (long)heap); //Subtract heap from block, shift, then add; 
	if(buddy->size != block->size) {
		return NULL;
	}
	return buddy;
}

/*
 * External function to handle merging of blocks via adding and removeBlock
 */
static int buddyMerge(metadata_t *block) {
	metadata_t* buddy = buddyFinder(block);
	if(buddy == NULL || buddy->in_use || block->size == SBRK_SIZE) {
		addToHead(block); //No available memory
		return 0;
	}
	blockRemove(buddy);
	metadata_t* newBlock;
	if(block < buddy) {
		newBlock = block;
	} else {
		newBlock = buddy;
	}
	newBlock->size = block->size * 2; //Once merged, size has doubled
	buddyMerge(newBlock); //Call again to check for bigger available blocks
	return 1;
}

/*
 * External function to split block
 */
int memSplit(int start, int end) {
	//Extensive error checking to return valid inputs and check for failure
	if(start == 0 || start > 7) {
		return 0;
	}
	if(freelist[start] == NULL) {
		return 0;
	}
	if(end < 0 || end > 7) {
		return 0;
	}
	if(start <= end) {
		return 0;
	}
	metadata_t *toSplit = removeHead(start);
	short childSize = (toSplit->size)/2;
	metadata_t *left = toSplit;
	metadata_t *right = (metadata_t *)((char *)toSplit + childSize);
	left->in_use = right->in_use = 0;
	left->prev = right->next = NULL;
	left->next = right;
	right->prev = left;
	left->size = right->size = childSize;
	freelist[start - 1] = left;
	memSplit(start - 1, end);
	return 1;
}


/**
 * Prints the freelist in a pretty format. Used for debugging
 * The size listed after the arrow is the size of the blocks linked
 * at that index UNLESS an incorrect sized block was placed there
 */
void print_freelist()
{
    int block = 16;
    int realsize = 0;
    int i;
    for (i = 0; i<8; i++)
    {
        realsize = block;
        int c = 0;
        metadata_t* ptr = freelist[i];
        while(ptr != NULL)
        {
            realsize = ptr->size;
            c++;
            ptr = ptr->next;
        }
        printf("freelist[%d] -> %d\n",c, realsize);
        block *= 2;
    }
    printf("\n");
}