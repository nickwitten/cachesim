/**
 * @author ECE 3058 TAs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cachesim.h"
#include "lrustack.h"

// Statistics you will need to keep track.
counter_t accesses = 0;     
counter_t traffic_i = 0;
counter_t traffic_d = 0;
counter_t writebacks = 0;   
counter_t g_misses_i = 0;   
counter_t g_misses_d = 0;   
counter_t g_hits_i = 0;     
counter_t g_hits_d = 0;     
counter_t l1_misses_i = 0;
counter_t l1_misses_d = 0;
counter_t l1_hits_i = 0;
counter_t l1_hits_d = 0;
counter_t l2_misses_i = 0;
counter_t l2_misses_d = 0;
counter_t l2_hits_i = 0;
counter_t l2_hits_d = 0;

/**
 * Function to perform a very basic log2. It is not a full log function, 
 * but it is all that is needed for this assignment. The <math.h> log
 * function causes issues for some people, so we are providing this. 
 * 
 * @param x is the number you want the log of.
 * @returns Techinically, floor(log_2(x)). But for this lab, x should always be a power of 2.
 */
int simple_log_2(int x) {
    int val = 0;
    while (x > 1) {
        x /= 2;
        val++;
    }
    return val; 
}

//  Here are some global variables you may find useful to get you started.
//      Feel free to add/change anyting here.
// L1 Caches
cache_set_t* l1_cache_i;   
cache_set_t* l1_cache_d;     
// L2 Cache
cache_set_t* l2_cache;     
// Cache Paramters
int block_size = 64;         
int l1_cache_size = 16*1024; 
int l1_num_sets;
int l1_num_offset_bits;
int l1_num_index_bits;
int l2_cache_size;
int l2_ways;               
int l2_num_sets; 
int l2_num_offset_bits;
int l2_num_index_bits;

/*
 * Function to initialize a cache.
 * @param num_sets is the number of caches sets
 * @param ways is the number of ways within a set
 * @return the cache structure pointer
 */
cache_set_t* init_cache(int num_sets, int ways) {
    cache_set_t* cache = (cache_set_t*)malloc(num_sets*sizeof(cache_set_t));
    for (int i=0; i < num_sets; i++) {
        cache[i] = (struct cache_set_t){ways};
        cache[i].stack = init_lru_stack(ways);
        // Initialize each block within cache set
        cache[i].blocks = (cache_block_t*)malloc(sizeof(cache_block_t)*ways);
        for (int j=0; j < ways; j++) {
            cache[i].blocks[j] = (struct cache_block_t){0, 0, 0};
        }
    }
    return cache;
}


/**
 * Function to intialize your cache simulator with the given cache parameters. 
 * Note that we will only input valid parameters and all the inputs will always 
 * be a power of 2.
 * 
 * @param _block_size is the block size in bytes
 * @param _cache_size is the cache size in bytes
 * @param _ways is the associativity
 */
void cachesim_init(int _block_size, int _cache_size, int _ways) {
    // Set l1 cache parameters
    l1_num_sets = l1_cache_size / block_size;
    l1_num_offset_bits = simple_log_2(block_size);
    l1_num_index_bits = simple_log_2(l1_num_sets);     // Number of index bits. 

    // Initialize l1 caches
    l1_cache_i = init_cache(l1_num_sets, 1);
    l1_cache_d = init_cache(l1_num_sets, 1);

    // Set l2 cache parameters
    l2_cache_size = _cache_size;
    l2_ways = _ways;
    int l2_set_size = block_size*l2_ways;
    l2_num_sets = (int)(l2_cache_size / l2_set_size);
    l2_num_offset_bits = simple_log_2(block_size);
    l2_num_index_bits = simple_log_2(l2_num_sets);

    // Initialize l2 cache
    l2_cache = init_cache(l2_num_sets, l2_ways);
}


/*
 * Function to search a cache set for a given index
 * @param cache_set is the set to search
 * @param id is a structure containing the address's index, offset, and tag
 * @return block_ind is the index of the block with the matching tag.  -1 if not found.
 */
int search_cache(cache_set_t* cache_set, const addr_id_t* id) {
    // Search for the tag inside the cache set, will remain -1 if not found
    int block_ind = -1;
    for (int i=0; i < cache_set->size; i++) {
        if (cache_set->blocks[i].tag == id->tag && cache_set->blocks[i].valid) {
            block_ind = i;
        }
    }
    return block_ind;
}

/*
 * Function to break an address into its components
 * @param addr is the physical address
 * @param num_offset_bits is the amount of bits used for offset
 * @param num_index_bits is the amount of bits used for the index
 * @return id is the structure containing the index, offset, and tag
 */
addr_id_t parse_addr(addr_t addr, int num_offset_bits, int num_index_bits) {
    // Parse the address into tag, index, and offset
    addr_id_t id;
    addr_t offset_mask = (1 << num_offset_bits) - 1;
    addr_t index_mask = (1 << num_index_bits) - 1;
    id.offset = addr & offset_mask;
    addr = addr >> num_offset_bits;
    id.index = addr & index_mask;
    id.tag = addr >> num_index_bits;
    return id;
}

/**
 * Function to handle cache misses. Grabs the lru block,
 * updates evicted_block with this, and then updates that block
 * with the new information.
 * @param cache_set is a pointer to a cache set struct
 * @param addr_id is a structure containing address components
 * @param evicted_block is a pointer to a block that should be set to the evicted block
 */
void replace(cache_set_t* cache_set, const addr_id_t* addr_id, cache_block_t* evicted_block) {
    // Grab the lru block
    int lru_ind = lru_stack_get_lru(cache_set->stack);
    *evicted_block = cache_set->blocks[lru_ind];
    cache_block_t block;
    // Replace the block
    block.tag = addr_id->tag;
    block.valid = 1;
    block.dirty = 0;
    // Place the updated block in the cache
    cache_set->blocks[lru_ind] = block;
    // Set it as the mru
    lru_stack_set_mru(cache_set->stack, lru_ind);
}

/*
 * Handles l1 replacements.  Checks if l2 needs to be written on dirty evicted block.
 * @param l1_set is the cache set to put the new block in
 * @param l1_addr_id is the address components
 * @param data is 1 if it is a data memory value or 0 if an instruction memory value.
 */
void l1_replace(cache_set_t* l1_set, const addr_id_t* l1_addr_id, int data) {
    cache_block_t evicted_block;
    replace(l1_set, l1_addr_id, &evicted_block);
    if (!evicted_block.valid) {
        // The evicted block was unused
        return;
    }
    //printf("L1 Data Evicted, Index: %i, Tag: %i\n", l1_addr_id->index, evicted_block.tag);
    // If block is dirty, write it to l2
    if (!evicted_block.dirty) {
        return;
    }
    addr_t evicted_address = evicted_block.tag << (l1_num_offset_bits + l1_num_index_bits);
    int evicted_index = l1_addr_id->index;
    evicted_address |= evicted_index << l1_num_offset_bits;
    addr_id_t l2_id = parse_addr(evicted_address, l2_num_offset_bits, l2_num_index_bits);
    for (int i=0; i<l2_ways; i++) {
        cache_block_t block = l2_cache[l2_id.index].blocks[i];
        if (block.tag == l2_id.tag && block.valid) {
            l2_cache[l2_id.index].blocks[i].dirty = 1;
            if (data) {
                traffic_d++;
            } else {
                traffic_i++;
            }
            //printf("Traffic Now\n");
        }
    }
}

/*
 * Handles l2 replacements.  Checks if l2 is evicting a dirty block for writeback.
 * Also checks if l1 invalidation needs to happen.
 * @param l2_set is the cache set to put the new block in
 * @param l2_addr_id is the address components
 */
void l2_replace(cache_set_t* l2_set, const addr_id_t* l2_addr_id) {
    cache_block_t evicted_block;
    replace(l2_set, l2_addr_id, &evicted_block);
    if (!evicted_block.valid) {
        // The evicted block was unused
        return;
    }
    if (evicted_block.dirty) {
        writebacks++;
    }
    //printf("L2 Data Evicted, Index: %i, Tag: %i\n", l2_addr_id->index, evicted_block.tag);
    // Check if its in L1 cache
    int evicted_index = l2_addr_id->index;
    addr_t evicted_address = evicted_block.tag << (l2_num_offset_bits + l2_num_index_bits);
    evicted_address |= evicted_index << l2_num_offset_bits;
    addr_id_t l1_id = parse_addr(evicted_address, l1_num_offset_bits, l1_num_index_bits);
    //printf("Searching for L1 Data, Index: %i, Tag: %i\n", l1_id.index, l1_id.tag);
    if (l1_cache_d[l1_id.index].blocks[0].tag == l1_id.tag && l1_cache_d[l1_id.index].blocks[0].valid) {
        //printf("Back invalidation\n");
        if (l1_cache_d[l1_id.index].blocks[0].dirty == 1) {
            writebacks++;
        }
        l1_cache_d[l1_id.index].blocks[0].valid = 0;
        //printf("Traffic Now\n");
        traffic_d++;
    }
    if (l1_cache_i[l1_id.index].blocks[0].tag == l1_id.tag && l1_cache_i[l1_id.index].blocks[0].valid) {
        //printf("Back invalidation\n");
        if (l1_cache_i[l1_id.index].blocks[0].dirty == 1) {
            writebacks++;
        }
        l1_cache_i[l1_id.index].blocks[0].valid = 0;
        //printf("Traffic Now\n");
        traffic_i++;
    }
}

/**
 * Function to handle a data read.
 * @param l1_addr_id is the components of the address for l1 cache
 * @param l2_addr_id is the components of the address for l2 cache
 */
void read_data_access(const addr_id_t* l1_addr_id, const addr_id_t* l2_addr_id) {
    cache_set_t* l1_set = l1_cache_d + l1_addr_id->index;
    cache_set_t* l2_set = l2_cache + l2_addr_id->index;
    int l1_block_ind = search_cache(l1_set, l1_addr_id);
    int l2_block_ind = search_cache(l2_set, l2_addr_id);
    if (l1_block_ind == -1) {
        if (l2_block_ind == -1) {
            //printf("L1 and L2 missed\n");
            // Both missed
            l1_replace(l1_set, l1_addr_id, 1);
            l2_replace(l2_set, l2_addr_id);
            l2_misses_d++;
            g_misses_d++;
        } else {
            //printf("L2 hit\n");
            // l2 hit
            l1_replace(l1_set, l1_addr_id, 1);
            lru_stack_set_mru(l2_set->stack, l2_block_ind);
            //printf("Traffic Now\n");
            traffic_d++;
            g_hits_d++;
            l2_hits_d++;
        }
        l1_misses_d++;
    } else {
        //printf("L1 hit\n");
        // l1 hit
        lru_stack_set_mru(l1_set->stack, l1_block_ind);
        // Update L2 LRU so that L1 doesn't recieve an
        // invalidation for this block later even if it's
        // used frequently.
        lru_stack_set_mru(l2_set->stack, l2_block_ind);
        g_hits_d++;
        l1_hits_d++;
    }
}

/**
 * Function to handle a data write.
 * @param l1_addr_id is the components of the address for l1 cache
 * @param l2_addr_id is the components of the address for l2 cache
 */
void write_data_access(const addr_id_t* l1_addr_id, const addr_id_t* l2_addr_id) {
    cache_set_t* l1_set = l1_cache_d + l1_addr_id->index;
    cache_set_t* l2_set = l2_cache + l2_addr_id->index;
    int l1_block_ind = search_cache(l1_set, l1_addr_id);
    int l2_block_ind = search_cache(l2_set, l2_addr_id);
    if (l1_block_ind == -1) {
        if (l2_block_ind == -1) {
            //printf("L1 and L2 missed\n");
            // Both missed
            l1_replace(l1_set, l1_addr_id, 1);
            l2_replace(l2_set, l2_addr_id);
            l1_set->blocks[l1_set->stack->order[0]].dirty = 1;
            l2_misses_d++;
            g_misses_d++;
        } else {
            //printf("L2 hit\n");
            // l2 hit
            l1_replace(l1_set, l1_addr_id, 1);
            l1_set->blocks[l1_set->stack->order[0]].dirty = 1;
            lru_stack_set_mru(l2_set->stack, l2_block_ind);
            //printf("Traffic Now\n");
            traffic_d++;
            g_hits_d++;
            l2_hits_d++;
        }
        l1_misses_d++;
    } else {
        //printf("L1 hit\n");
        // l1 hit
        lru_stack_set_mru(l1_set->stack, l1_block_ind);
        // Update L2 LRU so that L1 doesn't recieve an
        // invalidation for this block later even if it's
        // used frequently.
        lru_stack_set_mru(l2_set->stack, l2_block_ind);
        l1_set->blocks[l1_set->stack->order[0]].dirty = 1;
        g_hits_d++;
        l1_hits_d++;
    }
}

/**
 * Function to handle a data read.
 * @param l1_addr_id is the components of the address for l1 cache
 * @param l2_addr_id is the components of the address for l2 cache
 */
void read_instr_access(const addr_id_t* l1_addr_id, const addr_id_t* l2_addr_id) {
    cache_set_t* l1_set = l1_cache_i + l1_addr_id->index;
    cache_set_t* l2_set = l2_cache + l2_addr_id->index;
    int l1_block_ind = search_cache(l1_set, l1_addr_id);
    int l2_block_ind = search_cache(l2_set, l2_addr_id);
    if (l1_block_ind == -1) {
        if (l2_block_ind == -1) {
            //printf("L1 and L2 missed\n");
            // Both missed
            l1_replace(l1_set, l1_addr_id, 0);
            l2_replace(l2_set, l2_addr_id);
            l2_misses_i++;
            g_misses_i++;
        } else {
            //printf("L2 hit\n");
            // l2 hit
            l1_replace(l1_set, l1_addr_id, 0);
            lru_stack_set_mru(l2_set->stack, l2_block_ind);
            //printf("Traffic Now\n");
            traffic_i++;
            g_hits_i++;
            l2_hits_i++;
        }
        l1_misses_i++;
    } else {
        //printf("L1 hit\n");
        // l1 hit
        lru_stack_set_mru(l1_set->stack, l1_block_ind);
        // Update L2 LRU so that L1 doesn't recieve an
        // invalidation for this block later even if it's
        // used frequently.
        lru_stack_set_mru(l2_set->stack, l2_block_ind);
        g_hits_i++;
        l1_hits_i++;
    }
}

/**
 * Function to perform a SINGLE memory access to your cache. In this function, 
 * you will need to update the required statistics (accesses, hits, misses, writebacks)
 * and update your cache data structure with any changes necessary.
 * 
 * @param physical_addr is the address to use for the memory access. 
 * @param access_type is the type of access - 0 (data read), 1 (data write) or 
 *      2 (instruction read). We have provided macros (MEMREAD, MEMWRITE, IFETCH)
 *      to reflect these values in cachesim.h so you can make your code more readable.
 */
void cachesim_access(addr_t physical_addr, int access_type) {
    accesses++;  // Increment stat
    // Parse address for both levels of cache
    addr_id_t l1_addr_id = parse_addr(physical_addr, l1_num_offset_bits, l1_num_index_bits);
    addr_id_t l2_addr_id = parse_addr(physical_addr, l2_num_offset_bits, l2_num_index_bits);
    //printf("l1 index: %i, l1 tag: %i\n", l1_addr_id.index, l1_addr_id.tag);
    //printf("l2 index: %i, l2 tag: %i\n", l2_addr_id.index, l2_addr_id.tag);

    // Dispatch into different actions based on access type
    switch(access_type) {
        case MEMREAD:
            read_data_access(&l1_addr_id, &l2_addr_id);
            break;
        case MEMWRITE:
            write_data_access(&l1_addr_id, &l2_addr_id);
            break;
        case IFETCH:
            read_instr_access(&l1_addr_id, &l2_addr_id);
            break;
        default:
            break;
    }
    //printf("\n\n");
}

/*
 * Function to free an array of cache sets
 * @param cache is a pointer to the first cache set.
 * @param num_sets is the number of sets in the array.
 */
void free_cache(cache_set_t* cache, int num_sets) {
    // Free all blocks and stacks for each cache set
    for (int i=0; i < num_sets; i++) {
        free(cache[i].blocks);
        lru_stack_cleanup(cache[i].stack);
    }
    // Free the array of cache sets
    free(cache);
}

/**
 * Function to free up any dynamically allocated memory you allocated
 */
void cachesim_cleanup() {
    free_cache(l1_cache_i, l1_num_sets);
    free_cache(l1_cache_d, l1_num_sets);
    free_cache(l2_cache, l2_num_sets);
}

/*
 * Function to compute and print global and local missrates for data and instructions.
 * As well as traffic for Data.
 */
void data_insights() {
    double g_missrate_d = ((double)g_misses_d) / ((double)(g_misses_d + g_hits_d));
    double g_missrate_i = ((double)g_misses_i) / ((double)(g_misses_i + g_hits_i));
    double l1_missrate_d = ((double)l1_misses_d) / ((double)(l1_misses_d + l1_hits_d));
    double l1_missrate_i = ((double)l1_misses_i) / ((double)(l1_misses_i + l1_hits_i));
    double l2_missrate_d = ((double)l2_misses_d) / ((double)(l2_misses_d + l2_hits_d));
    double l2_missrate_i = ((double)l2_misses_i) / ((double)(l2_misses_i + l2_hits_i));
    printf("Global Data Missrate: %f\n", g_missrate_d);
    printf("Global Instruction Missrate: %f\n", g_missrate_i);
    printf("L1 Data Missrate: %f\n", l1_missrate_d);
    printf("L1 Instruction Missrate: %f\n", l1_missrate_i);
    printf("L2 Data Missrate: %f\n", l2_missrate_d);
    printf("L2 Instruction Missrate: %f\n", l2_missrate_i);
    printf("Traffic for D-Cache: %llu\n", traffic_d);
}

/**
 * Function to print cache statistics
 * DO NOT update what this prints.
 */
void cachesim_print_stats() {
    long long unsigned misses = g_misses_i + g_misses_d;
    long long unsigned hits = g_hits_i + g_hits_d;
    printf("%llu, %llu, %llu, %llu\n", accesses, hits, misses, writebacks);  
}

/**
 * Function to open the trace file
 * You do not need to update this function. 
 */
FILE *open_trace(const char *filename) {
    return fopen(filename, "r");
}

/**
 * Read in next line of the trace
 * 
 * @param trace is the file handler for the trace
 * @return 0 when error or EOF and 1 otherwise. 
 */
int next_line(FILE* trace) {
    if (feof(trace) || ferror(trace)) return 0;
    else {
        int t;
        unsigned long long address, instr;
        fscanf(trace, "%d %llx %llx\n", &t, &address, &instr);
        cachesim_access(address, t);
    }
    return 1;
}

/**
 * Main function. See error message for usage. 
 * 
 * @param argc number of arguments
 * @param argv Argument values
 * @returns 0 on success. 
 */
int main(int argc, char **argv) {
    FILE *input;

    if (argc != 5) {
        fprintf(stderr, "Usage:\n  %s <trace> <block size(bytes)>"
                        " <cache size(bytes)> <ways>\n", argv[0]);
        return 1;
    }
    
    input = open_trace(argv[1]);
    cachesim_init(atol(argv[2]), atol(argv[3]), atol(argv[4]));
    while (next_line(input));
    cachesim_print_stats();
    //data_insights();
    cachesim_cleanup();
    fclose(input);
    return 0;
}

