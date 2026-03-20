#include <stdio.h>
#include "dummy_main.h"

int dummy_main(int argc, char** argv) {
    printf("Job C is starting...\n");

    // Simulate processing with a CPU-intensive loop
    volatile unsigned long count = 0;
    for (unsigned long i = 0; i < 1200; i++) {
        count += i; // Simple operation to keep CPU busy
    }

    printf("Job C has finished. Count: %lu\n", count);
    return 0;
}
