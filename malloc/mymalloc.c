#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include<assert.h>

// block meta data structure
typedef struct block_meta{
	size_t size;
	struct block_meta* next; // pointer to next block
	int free; // indicates whether block is free
	int magic; // for debugging
} block_meta;

#define META_SIZE sizeof(block_meta)

// head of linked list
void* global_base = NULL;

// function to find free block from linked list
block_meta* find_free_block(block_meta** last, size_t size){
	block_meta* current = global_base;
	while (current && !(current->free && current->size >= size)){
		*last = current; // keep track of last visited block
		current = current->next;
	}
	return current;
}

block_meta* request_space(block_meta* last, size_t size){
	block_meta* block;
	block = sbrk(0); // does not allocate memory. Just returns the address that marks the end of the process’s heap memory.
	void* request = sbrk(size + META_SIZE); // grows the heap starting from 'block' and returns starting address
	assert((void*)block == request);
	if (request == (void*) -1){
		return NULL; // sbrk failed
	}

	if (last){ // NULL on first request
		last->next = block;
	}
	block->size = size;
	block->free = 0;
	block->next = NULL;
	block->magic = 0x12345678;

	return block;
}

void* malloc(size_t size){
	block_meta* block;

	if (size < 0){
		return NULL;
	}

	if (!global_base){ // first call
		block = request_space(NULL, size);
		if (!block){
			return NULL;
		}
		global_base = block;
	}
	else {
		block_meta* last = global_base;
		block = find_free_block(&last, size);
		if (!block){ // failed to find free block in list
			block = request_space(last, size);
		       	if (!block) {
				return NULL;
			}
		}
		else { // found free block
		       block->free = 0;
		       block->magic = 0x77777777;
		}
	}

	// skips over the metadata and points to the start of the user-accessible memory
	return(block + 1);
}

block_meta* get_block_ptr(void* ptr){
	return (block_meta*)ptr - 1;
}

void free(void* ptr){
	if (!ptr){
		return;
	}

	block_meta* block_ptr = get_block_ptr(ptr);
	assert(block_ptr->free == 0);
	assert(block_ptr->magic == 0x77777777 || block_ptr->magic == 0x12345678);
	block_ptr->free = 1;
	block_ptr->magic = 0x55555555;
}

void* realloc(void* ptr, size_t size){
	if (!ptr){
		return malloc(size); // if NULL, act like malloc
	}

	block_meta* block = get_block_ptr(ptr);
	if (block->size >= size){ // we have enough space
		return ptr;
	}

	// if not, we need to really re-allocate memory
	// malloc new space, copy data to new space and free old space
	void* new_ptr = malloc(size);
	if (!new_ptr){
		return NULL;
	}
	memcpy(new_ptr, ptr, block->size);
	free(ptr);
	return new_ptr;
}

void* calloc(size_t num, size_t size){
	size_t tot_size = num * size;
	void* ptr = malloc(tot_size);
	memset(ptr, 0, tot_size);
	return ptr;
}

int main(){
    	printf("Allocating 25 bytes...\n");
    	char* a = (char*) malloc(25);
    	strcpy(a, "custom malloc works!");
    	printf("a: %s\n", a);
	printf("Address of a: %p\n", (void*)a);

    	printf("Allocating 30 bytes...\n");
    	char* b = (char*) malloc(30);
    	strcpy(b, "second block");
    	printf("b: %s\n", b);
	printf("Address of b: %p\n", (void*)b);

    	printf("Freeing a...\n");
    	free(a);

    	printf("Allocating 20 bytes (should reuse a)...\n");
    	char* c = (char*) malloc(20);
    	strcpy(c, "reused block");
    	printf("c: %s\n", c);
	printf("a: %s\n", a);
	printf("Address of c: %p\n", (void*)c);
						
	printf("Allocating zero-initialized array with calloc...\n");
    	int* arr = (int*) calloc(5, sizeof(int));
    	printf("calloc'd array: ");
    	for (int i = 0; i < 5; i++) {
    	    printf("%d ", arr[i]);  // should all be 0
    	}
    	printf("\n");
    	printf("Address of arr: %p\n", (void*)arr);
	
	printf("Reallocating block b from 30 bytes to 50...\n");
    	char* b_re = (char*) realloc(b, 50);
    	strcat(b_re, " extended!");
    	printf("Reallocated b: %s\n", b_re);
    	printf("New address of b: %p\n", (void*)b_re);

    	printf("Done.\n");
    	return 0;
}
