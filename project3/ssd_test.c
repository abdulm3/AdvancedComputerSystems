#include <stdio.h>
#include <stdlib.h>

// Function to generate and execute FIO command with sudo
void generate_fio_test(int block_size, int rw_ratio, int iodepth) {
    char command[512];
    
    // Construct the FIO command string with sudo
    sprintf(command, 
            "sudo fio --name=test --ioengine=libaio --direct=1 --time_based --runtime=60s "
            "--filename=/dev/nvme0n1p6 --blocksize=%dk --rw=randrw --rwmixread=%d "
            "--iodepth=%d --size=2G --allow_mounted_write=1",
            block_size, rw_ratio, iodepth);
    
    // Execute the FIO command with sudo
    system(command);
}

int main() {
    // Arrays for block sizes, read/write ratios, and queue depths
    int block_sizes[] = {4, 16, 32, 128};    // Block sizes in KB
    int rw_ratios[] = {100, 0, 50, 70};      // Read/Write ratios (100% read, 100% write, etc.)
    int queue_depths[] = {1, 32, 256, 1024}; // Queue depths

    // Nested loops to run FIO tests for all combinations
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                generate_fio_test(block_sizes[i], rw_ratios[j], queue_depths[k]);
            }
        }
    }

    return 0;
}
