#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GIGABYTE (1024 * 1024 * 1024)


//start main code
int main(int argc, char *argv[]) { if (argc != 3) 
    {
        fprintf(stderr, "Usage: %s <elementSize(bytes)>, %s <readRatio>\n", argv[0], argv[1]);
        return 1;
    }
    
    // Get element size from arguement 1
    size_t elementSize = (size_t)atoi(argv[1]); //affirm elementSize is valid
    if (elementSize == 0) 
    {
        fprintf(stderr, "Invalid element size. Must be a positive integer.\n");
        return 1;
    }

    // Get read ratio from arguement 2
    size_t readRatio = (size_t)atoi(argv[2]);
    if (readRatio <= 0 || readRatio > 100) //affirm readRatio is valid
    {
      fprintf(stderr, "Invalid read ratio. Must be an integer between 0 and 100.\n");
      return 1;
    }

    // numElements is the number of elements that fit in 1GB
    size_t numElements = GIGABYTE / elementSize;

    // Allocate 1GB mem
    void *memory = malloc(GIGABYTE);
    if (memory == NULL) 
    {
        fprintf(stderr, "Failed memory allocation\n");
        return 1;
    }

    size_t readElements = numElements * (readRatio / 100);

    // Write to mem
    for (size_t i = 0; i < numElements; i++) 
    {
        memset((char *)memory + i * elementSize, (char)(i % 256), elementSize);
    }
    printf("Completed memory write (element size: %zu bytes).\n", elementSize);

    // Reading from memory
    size_t errorCount = 0;
    for (size_t i = 0; i < readElements; i++) 
    {
        char *element = (char *)memory + i * elementSize;
        if (*element != (char)(i % 256)) {
            fprintf(stderr, "Memory error at element %zu\n", i);
            errorCount++;
            if (errorCount > 10) {
                break;  // Stop after finding 10 errors
            }
        }
    }
    printf("Completed memory read\n");
    free(memory); 
    printf("Memory freed.\n");

    return 0;
}
