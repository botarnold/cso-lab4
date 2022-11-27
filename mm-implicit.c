// This file gives you a starting point to implement malloc using implicit list
// Each chunk has a header (of type header_t) and does *not* include a footer
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "mm-common.h"
#include "mm-implicit.h"
#include "memlib.h"

// turn "debug" on while you are debugging correctness. 
// Turn it off when you want to measure performance
static bool debug = true; 

size_t hdr_size = sizeof(header_t);

void 
init_chunk(header_t *p, size_t csz, bool allocated)
{
	p->size = csz;
	p->allocated = allocated;
}

// Helper function next_chunk returns a pointer to the header of 
// next chunk after the current chunk h.
// It returns NULL if h is the last chunk on the heap.
// If h is NULL, next_chunk returns the first chunk if heap is non-empty, and NULL otherwise.
header_t *
next_chunk(header_t *h)
{
	//Your code here
	if(h==NULL){
		header_t *temp = (header_t *)mem_heap_lo();
		if (temp->allocated==false){
			return NULL;
		}
		else{
			return (header_t *)mem_heap_lo();
		}
		
	}
	else{
		header_t *next = (header_t *)((char *)h + h->size);
		if(next->allocated==0){
			if ((char *)next>=(char *)mem_heap_hi()){
				return NULL;
			}
			else{
				return next;
			}
		}
		else{
			return next;
		}
	}
}


/* 
 * mm_init initializes the malloc package.
 */
int mm_init(void)
{
	//double check that hdr_size should be 16-byte aligned
	assert(hdr_size == align(hdr_size));
	// start with an empty heap. 
	// no additional initialization code is necessary for implicit list.
	return 0;
}


// helper function first_fit traverses the entire heap chunk by chunk from the begining. 
// It returns the first free chunk encountered whose size is bigger or equal to "csz".  
// It returns NULL if no large enough free chunk is found.
// Please use helper function next_chunk when traversing the heap
header_t *
first_fit(size_t csz)
{
	//Your code here
	header_t *curr = (header_t *)mem_heap_lo();
	while ((char *)curr<(char *)mem_heap_hi()){
		if(curr->allocated==false){
			if(curr->size>=csz){
				return curr;
			}
		}
		else
		{
			curr = next_chunk(curr);
			if(curr==NULL){
				return NULL;
			}
		}
	}
	return NULL;
}

// helper function split cuts the chunk into two chunks. The first chunk is of size "csz", 
// the second chunk contains the remaining bytes. 
// You must check that the size of the original chunk is big enough to enable such a cut.
void
split(header_t *original, size_t csz)
{
	//Your code here
	size_t o_size = original->size; 
	if (o_size>=csz){
		size_t remain = o_size - csz; 
		original->size = csz;
		header_t *second_chunk  = (header_t *)((char *)original + original->size);
		//header_t *second_chunk;
		//second_chunk = (header_t *)mem_sbrk(remain);
		init_chunk(second_chunk, remain, false);
		second_chunk->size = remain;
		second_chunk->allocated = false;
				
	}
}

// helper function ask_os_for_chunk invokes the mem_sbrk function to ask for a chunk of 
// memory (of size csz) from the "operating system". It initializes the new chunk 
// using helper function init_chunk and returns the initialized chunk.
header_t *
ask_os_for_chunk(size_t csz)
{
	//Your code here
	header_t *p;
	p = (header_t *)mem_sbrk(csz);
	init_chunk(p, csz, false);
	p->size = csz;
	p->allocated = false;
	return p;
}

/* 
 * mm_malloc allocates a memory block of size bytes
 */
void *
mm_malloc(size_t size)
{
	//make requested payload size aligned
	size = align(size);
	//chunk size is aligned because both payload and header sizes
	//are aligned
	size_t csz = hdr_size + align(size);

	header_t *p = NULL;

	//Your code here 
	//to obtain a free chunk p to satisfy this request.
	//
	//The code logic should be:
	//Try to find a free chunk using helper function first_fit
	//    If found, split the chunk (using helper function split).
	//    If not found, ask OS for new memory using helper ask_os_for_chunk
	//Set the chunk's status to be allocated
//	printf("\nfirst fit\n");
	p = first_fit(csz);
//	printf("initial: %p\n", p);
//	printf("psize: %ld\n", p->size);
	if(p==NULL){
//		printf("NULL\n");
		p = ask_os_for_chunk(csz);
	}
	else{
//		printf("Not NULL\n");
		split(p, csz);
	} 
	p->allocated = true;

	p = (header_t *)((char *)p + hdr_size);
//	heap_info_t info = mm_checkheap(true);
//	printf("address: %p\n", (char *)p);
//	printf("free chunk: %ld\nfree size: %ld\n", info.num_free_chunks, info.free_size);
//	printf("allo chunk: %ld\nallo size: %ld\n", info.num_allocated_chunks, info.allocated_size); 
	//After finishing obtaining free chunk p, 
	//check heap correctness to catch bugs
	if (debug) {
		mm_checkheap(true);
	}
	return (void *)p;
}

// Helper function payload_to_header returns a pointer to the 
// chunk header given a pointer to the payload of the chunk 
header_t *
payload2header(void *p)
{
	//Your code here
	return (header_t *)((char *)p - hdr_size);
}

// Helper function coalesce merges free chunk h with subsequent 
// consecutive free chunks to become one large free chunk.
// You should use next_chunk when implementing this function
void
coalesce(header_t *h)
{
	//Your code here
	header_t *temp;
	bool flag = true;
	while (flag){
		temp = next_chunk(h);
		if (temp==NULL){
			flag=false;
		}
		else{
			size_t allocated = temp->allocated;
			if (allocated==true){
				flag=false;
			}
			else{
				h->size += temp->size;
				temp = NULL;
				//temp->size = 0;
			}
		}
	}

}

/*
 * mm_free frees the previously allocated memory block
 */
void 
mm_free(void *p)
{
	// Your code here
	// 
	// The code logic should be:
	// Obtain pointer to current chunk using helper payload_to_header 
	// Set current chunk status to "free"
	// Call coalesce() to merge current chunk with subsequent free chunks
	header_t *curr = payload2header(p);
	curr->allocated = false;
	coalesce(curr);
	  
	// After freeing the chunk, check heap correctness to catch bugs
	if (debug) {
		mm_checkheap(true);
	}
}	

/*
 * mm_realloc changes the size of the memory block pointed to by ptr to size bytes.  
 * The contents will be unchanged in the range from the start of the region up to the minimum of   
 * the  old  and  new sizes.  If the new size is larger than the old size, the added memory will   
 * not be initialized.  If ptr is NULL, then the call is equivalent  to  malloc(size).
 * if size is equal to zero, and ptr is not NULL, then the call is equivalent to free(ptr).
 */
void *
mm_realloc(void *ptr, size_t size)
{
	// Your code here
	printf("\n");
	printf("initial: %p, %ld\n", ptr, sizeof(ptr));
	if (ptr==NULL){
		printf("NULL\n");
		mm_malloc(size);
	}
	else{
		printf("NOT NULL\n");
		if (size==0){
			printf("size==0\n");
			free(ptr);
		}
		else{
			printf("size!=0\n");
			ptr = payload2header(ptr);
			printf("after payload: %p, %ld\n", ptr, sizeof(ptr));
			coalesce(ptr);
			printf("after: %p, %ld\n", ptr, sizeof(ptr));
			//coalesce(ptr);
//			ptr = (char *)ptr + hdr_size;
			ptr += hdr_size;
			printf("final: %p, %ld\n", ptr, sizeof(ptr));
			mm_checkheap(true);
			
		}
	}	  
	// Check heap correctness after realloc to catch bugs
	if (debug) {
		mm_checkheap(true);
	}
	return ptr;
}


/*
 * mm_checkheap checks the integrity of the heap and returns a struct containing 
 * basic statistics about the heap. Please use helper function next_chunk when 
 * traversing the heap
 */
heap_info_t 
mm_checkheap(bool verbose) 
{
	heap_info_t info;
	// Your code here
	// 
	// traverse the heap to fill in the fields of info
	info.num_allocated_chunks = 0;
	info.num_free_chunks = 0;
	info.allocated_size = 0;
	info.free_size = 0;
	header_t *curr = (header_t *)mem_heap_lo();
	size_t remain_size = mem_heapsize(); 
	while((char *)curr < (char *)mem_heap_hi()){
		size_t allocated = curr->allocated;
		size_t csz = curr->size;
		if (allocated==1) {
			info.num_allocated_chunks += 1;
			info.allocated_size += csz;
			remain_size -= csz;
			}
		else{
			info.num_free_chunks += 1;
			info.free_size += csz;
			remain_size -=csz;
		}
		curr = next_chunk(curr);
		if (curr==NULL){
			//info.free_size += remain_size;
			break;
		}
	}
	// debug mode
	if(verbose){
		printf("\nallocated: %ld, %ld\nfree: %ld, %ld\n", info.num_allocated_chunks, info.allocated_size, info.num_free_chunks, info.free_size);
	}
	// correctness of implicit heap amounts to the following assertion.
	assert(mem_heapsize() == (info.allocated_size + info.free_size));
	return info;
}
