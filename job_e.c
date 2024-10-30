#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main() {
    // Get the start time
    clock_t start_time = clock();

    // Run the loop until 200 ms have passed
    while ((clock() - start_time) * 1000 / CLOCKS_PER_SEC < 200) {
        // Busy-waiting for 200 ms
    }

    printf("Job completed.\n");
    return 0;
}
