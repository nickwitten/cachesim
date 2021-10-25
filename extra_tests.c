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

void reset() {
    accesses = 0;     // Total number of cache accesses
    traffic = 0;
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
 * L1 hits
 */
void test1() {
    FILE *input;
    input = open_trace("./Traces/extra_test1.txt");
    cachesim_init(64, 262144, 8);
    while (next_line(input));
    cachesim_cleanup();
    fclose(input);
    assert_equal(1, 4, accesses);
    assert_equal(2, 0, traffic);
    assert_equal(3, 0, writebacks);

    assert_equal(4, 1, g_hits_i);
    assert_equal(5, 1, g_hits_d);
    assert_equal(6, 1, g_misses_i);
    assert_equal(7, 1, g_misses_d);

    assert_equal(8, 1, l1_hits_i);
    assert_equal(9, 1, l1_hits_d);
    assert_equal(10, 1, l1_misses_i);
    assert_equal(11, 1, l1_misses_d);

    assert_equal(12, 0, l2_hits_i);
    assert_equal(13, 0, l2_hits_d);
    assert_equal(14, 1, l2_misses_i);
    assert_equal(15, 1, l2_misses_d);
    reset();
}

/*
 * l1 miss and l2 hit
 */
void test2() {
    FILE *input;
    input = open_trace("./Traces/extra_test2.txt");
    cachesim_init(64, 262144, 8);
    while (next_line(input));
    cachesim_cleanup();
    fclose(input);
    assert_equal(1, 8, accesses);
    assert_equal(2, 4, traffic);
    assert_equal(3, 0, writebacks);
                  
    assert_equal(4, 2, g_hits_i);
    assert_equal(5, 2, g_hits_d);
    assert_equal(6, 2, g_misses_i);
    assert_equal(7, 2, g_misses_d);
                  
    assert_equal(8, 0, l1_hits_i);
    assert_equal(9, 0, l1_hits_d);
    assert_equal(10, 4, l1_misses_i);
    assert_equal(11, 4, l1_misses_d);
                  
    assert_equal(12, 2, l2_hits_i);
    assert_equal(13, 2, l2_hits_d);
    assert_equal(14, 2, l2_misses_i);
    assert_equal(15, 2, l2_misses_d);
    reset();
}
    
/*
 * Write in l1, evict and write in l2, evict and writeback.
 */
void test3() {
    FILE *input;
    input = open_trace("./Traces/extra_test3.txt");
    cachesim_init(64, 262144, 8);
    while (next_line(input));
    cachesim_cleanup();
    fclose(input);
    assert_equal(1, 9, accesses);
    assert_equal(2, 1, traffic);
    assert_equal(3, 1, writebacks);
                  
    assert_equal(4, 0, g_hits_i);
    assert_equal(5, 0, g_hits_d);
    assert_equal(6, 0, g_misses_i);
    assert_equal(7, 9, g_misses_d);
                  
    assert_equal(8, 0, l1_hits_i);
    assert_equal(9, 0, l1_hits_d);
    assert_equal(10, 0, l1_misses_i);
    assert_equal(11, 9, l1_misses_d);
                  
    assert_equal(12, 0, l2_hits_i);
    assert_equal(13, 0, l2_hits_d);
    assert_equal(14, 0, l2_misses_i);
    assert_equal(15, 9, l2_misses_d);
    reset();
}

/*
 * Instruction in l1, evict from l2, invalidation l1.
 */
void test4() {
    FILE *input;
    input = open_trace("./Traces/extra_test4.txt");
    cachesim_init(64, 262144, 8);
    while (next_line(input));
    cachesim_cleanup();
    fclose(input);
    assert_equal(1, 10, accesses);
    assert_equal(2, 1, traffic);
    assert_equal(3, 0, writebacks);
                  
    assert_equal(4, 0, g_hits_i);
    assert_equal(5, 0, g_hits_d);
    assert_equal(6, 1, g_misses_i);
    assert_equal(7, 9, g_misses_d);
                  
    assert_equal(8, 0, l1_hits_i);
    assert_equal(9, 0, l1_hits_d);
    assert_equal(10, 1, l1_misses_i);
    assert_equal(11, 9, l1_misses_d);
                  
    assert_equal(12, 0, l2_hits_i);
    assert_equal(13, 0, l2_hits_d);
    assert_equal(14, 1, l2_misses_i);
    assert_equal(15, 9, l2_misses_d);
    reset();
}

/*
 * Write in l1, evict from l2, invalidation and writeback in l1..
 */
void test5() {
    FILE *input;
    input = open_trace("./Traces/extra_test5.txt");
    cachesim_init(64, 65536, 8);
    while (next_line(input));
    cachesim_cleanup();
    fclose(input);
    assert_equal(1, 9, accesses);
    assert_equal(2, 1, traffic);
    assert_equal(3, 1, writebacks);
                  
    assert_equal(4, 0, g_hits_i);
    assert_equal(5, 0, g_hits_d);
    assert_equal(6, 0, g_misses_i);
    assert_equal(7, 9, g_misses_d);
                  
    assert_equal(8, 0, l1_hits_i);
    assert_equal(9, 0, l1_hits_d);
    assert_equal(10, 0, l1_misses_i);
    assert_equal(11, 9, l1_misses_d);
                  
    assert_equal(12, 0, l2_hits_i);
    assert_equal(13, 0, l2_hits_d);
    assert_equal(14, 0, l2_misses_i);
    assert_equal(15, 9, l2_misses_d);
    reset();
}

int main() {
    printf("------- Running tests -------\n");
    printf("------- L1 Hits Test  -------\n");
    test1();
    printf("\n------- L2 Hits Test  -----\n");
    test2();
    printf("------- L1 Write, L1 Evict, L2 Evict --------\n");
    test3();
    printf("------- L1 Write, L2 Evict and L1 Invalidate --------\n");
    test4();
    printf("------- L1 Write, L2 Evict and L1 Invalidate --------\n");
    test5();
    return 0;
}
