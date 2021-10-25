/**
 * @author ECE 3058 TAs
 */

#include <stdio.h>
#include <stdlib.h>
#include "cachesim.h"
#include "lrustack.h"

/**
 * Function to assert equality for test cases
 * 
 * @param test_num number of this test
 * @param expected the expected value
 * @param actual the actual value
 */
void assert_equal(int test_num, int expected, int actual) {
    if (expected == actual) {
        printf("Test%d Succeeded!\n", test_num);
    } else {
        printf("Test%d Failed: expected %d, but got %d\n", test_num, expected, actual);
    }
}

/*
 * Function to reset stats between tests
 */
void reset() {
    accesses = 0;     // Total number of cache accesses
    traffic_i = 0;
    traffic_d = 0;
    writebacks = 0;   // Total number of writebacks
    g_misses_i = 0;       // Total number of cache misses
    g_misses_d = 0;       // Total number of cache misses
    g_hits_i = 0;       // Total number of cache misses
    g_hits_d = 0;       // Total number of cache misses
    l1_misses_i = 0;
    l1_misses_d = 0;
    l1_hits_i = 0;
    l1_hits_d = 0;
    l2_misses_i = 0;
    l2_misses_d = 0;
    l2_hits_i = 0;
    l2_hits_d = 0;
}

/*
 * Function to test L1 hits.  Reads a data and instruction value twice each.
 * The second should result in an l1 hit for each.
 */
void test1() {
    printf("------- L1 Hits  -------\n");
    FILE *input;
    input = open_trace("./Traces/extra_test1.txt");
    cachesim_init(64, 262144, 8);
    while (next_line(input));
    cachesim_cleanup();
    fclose(input);
    assert_equal(1, 4, accesses);
    assert_equal(2, 0, traffic_i);
    assert_equal(3, 0, traffic_d);
    assert_equal(4, 0, writebacks);

    assert_equal(5, 1, g_hits_i);
    assert_equal(6, 1, g_hits_d);
    assert_equal(7, 1, g_misses_i);
    assert_equal(8, 1, g_misses_d);

    assert_equal(9, 1, l1_hits_i);
    assert_equal(10, 1, l1_hits_d);
    assert_equal(11, 1, l1_misses_i);
    assert_equal(12, 1, l1_misses_d);

    assert_equal(13, 0, l2_hits_i);
    assert_equal(14, 0, l2_hits_d);
    assert_equal(15, 1, l2_misses_i);
    assert_equal(16, 1, l2_misses_d);
    reset();
}

/*
 * Function to test L2 hits.  Reads in 2 instructions and 2 data values with the same
 * L1 index.  In the second round, they should all hit in L2 but not L1.
 */
void test2() {
    printf("\n------- L2 Hits  -----\n");
    FILE *input;
    input = open_trace("./Traces/extra_test2.txt");
    cachesim_init(64, 262144, 8);
    while (next_line(input));
    cachesim_cleanup();
    fclose(input);
    assert_equal(1, 8, accesses);
    assert_equal(2, 2, traffic_i);
    assert_equal(3, 2, traffic_d);
    assert_equal(4, 0, writebacks);
                   
    assert_equal(5, 2, g_hits_i);
    assert_equal(6, 2, g_hits_d);
    assert_equal(7, 2, g_misses_i);
    assert_equal(8, 2, g_misses_d);
                   
    assert_equal(9, 0, l1_hits_i);
    assert_equal(10, 0, l1_hits_d);
    assert_equal(11, 4, l1_misses_i);
    assert_equal(12, 4, l1_misses_d);
                   
    assert_equal(13, 2, l2_hits_i);
    assert_equal(14, 2, l2_hits_d);
    assert_equal(15, 2, l2_misses_i);
    assert_equal(16, 2, l2_misses_d);
    reset();
}
    
/*
 * Function to test L1 eviction of dirty data.  Writes to an index in L1, then reads
 * data 7 times to that same index, which will fill the L2 cache set as well as evict
 * from L1.  L2 should contain a dirty LRU block.  One final data is brought into L2,
 * where this data should be written back to memory.
 */
void test3() {
    printf("------- L1 Write, L1 Evict, L2 Evict and Writeback --------\n");
    FILE *input;
    input = open_trace("./Traces/extra_test3.txt");
    cachesim_init(64, 262144, 8);
    while (next_line(input));
    cachesim_cleanup();
    fclose(input);
    assert_equal(1, 9, accesses);
    assert_equal(2, 0, traffic_i);
    assert_equal(3, 1, traffic_d);
    assert_equal(4, 1, writebacks);
                   
    assert_equal(5, 0, g_hits_i);
    assert_equal(6, 0, g_hits_d);
    assert_equal(7, 0, g_misses_i);
    assert_equal(8, 9, g_misses_d);
                   
    assert_equal(9, 0, l1_hits_i);
    assert_equal(10, 0, l1_hits_d);
    assert_equal(11, 0, l1_misses_i);
    assert_equal(12, 9, l1_misses_d);
                   
    assert_equal(13, 0, l2_hits_i);
    assert_equal(14, 0, l2_hits_d);
    assert_equal(15, 0, l2_misses_i);
    assert_equal(16, 9, l2_misses_d);
    reset();
}

/*
 * Function to test invalidation.  Reads in an instruction.  Then reads in data with 
 * with the same L2 index 8 times to evict the instruction address.  L2 should 
 * invalidate the instruction that is still in the L1 instruction cache.
 */
void test4() {
    printf("------- L1 I Type, Invalidation Due to Data Filling L2 Set --------\n");
    FILE *input;
    input = open_trace("./Traces/extra_test4.txt");
    cachesim_init(64, 262144, 8);
    while (next_line(input));
    cachesim_cleanup();
    fclose(input);
    assert_equal(1, 10, accesses);
    assert_equal(2, 1, traffic_i);
    assert_equal(3, 0, traffic_d);
    assert_equal(4, 0, writebacks);
                   
    assert_equal(5, 0, g_hits_i);
    assert_equal(6, 0, g_hits_d);
    assert_equal(7, 1, g_misses_i);
    assert_equal(8, 9, g_misses_d);
                   
    assert_equal(9, 0, l1_hits_i);
    assert_equal(10, 0, l1_hits_d);
    assert_equal(11, 1, l1_misses_i);
    assert_equal(12, 9, l1_misses_d);
                   
    assert_equal(13, 0, l2_hits_i);
    assert_equal(14, 0, l2_hits_d);
    assert_equal(15, 1, l2_misses_i);
    assert_equal(16, 9, l2_misses_d);
    reset();
}

/*
 * Function to test dirty L1 invalidation as well as invalidation due to L2 index size.
 * Writes to an instruction.  Then reads in 8 data values with different L1 index values from
 * the first write but the same L2 index value as the first write.  
 * This will evict the first memory address from L2, send an invalidation to L1, and cause 
 * a writeback due to it being dirty.
 */
void test5() {
    printf("------- L1 Write, Eviction from L2 and Invalidation Writeback --------\n");
    FILE *input;
    input = open_trace("./Traces/extra_test5.txt");
    cachesim_init(64, 65536, 8);
    while (next_line(input));
    cachesim_cleanup();
    fclose(input);
    assert_equal(1, 9, accesses);
    assert_equal(2, 0, traffic_i);
    assert_equal(3, 1, traffic_d);
    assert_equal(4, 1, writebacks);
                   
    assert_equal(5, 0, g_hits_i);
    assert_equal(6, 0, g_hits_d);
    assert_equal(7, 0, g_misses_i);
    assert_equal(8, 9, g_misses_d);
                   
    assert_equal(9, 0, l1_hits_i);
    assert_equal(10, 0, l1_hits_d);
    assert_equal(11, 0, l1_misses_i);
    assert_equal(12, 9, l1_misses_d);
                   
    assert_equal(13, 0, l2_hits_i);
    assert_equal(14, 0, l2_hits_d);
    assert_equal(15, 0, l2_misses_i);
    assert_equal(16, 9, l2_misses_d);
    reset();
}

/*
 * Main function to run each test
 */
int main() {
    printf("------- Running tests -------\n");
    test1();
    test2();
    test3();
    test4();
    test5();
    return 0;
}
