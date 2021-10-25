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
counter_t accesses;     
counter_t traffic_i;
counter_t traffic_d;
counter_t writebacks;   
counter_t g_misses_i;   
counter_t g_misses_d;   
counter_t g_hits_i;     
counter_t g_hits_d;     
counter_t l1_misses_i;
counter_t l1_misses_d;
counter_t l1_hits_i;
counter_t l1_hits_d;
counter_t l2_misses_i;
counter_t l2_misses_d;
counter_t l2_hits_i;
counter_t l2_hits_d;


int simple_log_2(int x);
cache_set_t* init_cache(int num_sets, int ways);
void cachesim_init(int block_size, int cache_size, int ways);
int search_cache(cache_set_t* cache_set, const addr_id_t* id);
addr_id_t parse_addr(addr_t addr, int num_offset_bits, int num_index_bits);
void replace(cache_set_t* cache_set, const addr_id_t* addr_id, cache_block_t* evicted_block);
void l1_replace(cache_set_t* l1_set, const addr_id_t* l1_addr_id, int data);
void l2_replace(cache_set_t* l2_set, const addr_id_t* l2_addr_id);
void read_data_access(const addr_id_t* l1_addr_id, const addr_id_t* l2_addr_id);
void write_data_access(const addr_id_t* l1_addr_id, const addr_id_t* l2_addr_id);
void read_instr_access(const addr_id_t* l1_addr_id, const addr_id_t* l2_addr_id);
void cachesim_access(addr_t physical_add, int access_type);
void free_cache(cache_set_t* cache, int num_sets);
void cachesim_cleanup(void);
void cachesim_print_stats(void);
FILE *open_trace(const char *filename);
int next_line(FILE* trace);

#endif
