/**
 * @author ECE 3058 TAs
 */

#ifndef __CACHESIM_H
#define __CACHESIM_H

// Use these in code to make mem read/mem write/inst read related code more readable
#define MEMREAD 0
#define MEMWRITE 1
#define IFETCH 2

#include "lrustack.h"

// Please DO NOT CHANGE the following two typedefs
typedef unsigned long long addr_t;		// Data type to hold addresses
typedef unsigned long long counter_t;	// Data type to hold cache statistic variables

/**
 * Struct for a cache block. Feel free to change any of this if you want. 
 */
typedef struct cache_block_t {
	int tag;
	int valid;
	int dirty;
} cache_block_t;

/**
 * Struct for a cache set. Feel free to change any of this if you want. 
 */
typedef struct cache_set_t {
	int size;				// Number of blocks in this cache set
	lru_stack_t* stack;		// LRU Stack 
	cache_block_t* blocks;	// Array of cache block structs. You will need to
							// 	dynamically allocate based on number of blocks
							//	per set. 
} cache_set_t;

typedef struct addr_id_t {
    int tag;
    int index;
    int offset;
} addr_id_t;

/*
 * Global stats variables
 */
counter_t accesses;     // Total number of cache accesses
counter_t traffic;
counter_t writebacks;   // Total number of writebacks
counter_t g_misses_i;       // Total number of cache misses
counter_t g_misses_d;       // Total number of cache misses
counter_t g_hits_i;       // Total number of cache misses
counter_t g_hits_d;       // Total number of cache misses
counter_t l1_misses_i;
counter_t l1_misses_d;
counter_t l1_hits_i;
counter_t l1_hits_d;
counter_t l2_misses_i;
counter_t l2_misses_d;
counter_t l2_hits_i;
counter_t l2_hits_d;


void cachesim_init(int block_size, int cache_size, int ways);
FILE *open_trace(const char *filename);
int next_line(FILE* trace);
void l2_miss(cache_set_t* cache_set, int tag);
int search_cache(cache_set_t* cache, const addr_id_t* id);
void read_data_access();
void write_data_access();
void read_instr_access();
void cachesim_access(addr_t physical_add, int access_type);
void cachesim_cleanup(void);
void cachesim_print_stats(void);

#endif
