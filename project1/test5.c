#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define ARRAY_SIZE 100000000  // Size of the array (Changes between test4 and test5)

int array[ARRAY_SIZE];  // The array that will be accessed

// Cache Friendly Access (Sequential)
void cache_friendly_access(int *arr, int size) 
{
    int sum = 0;
    for (int i = 0; i < size; i++) 
    {
        sum += arr[i];
    }
    printf("The cache-friendly sum: %d\n", sum);
}

// Cache Hostile Access (Random)
void cache_hostile_access(int *arr, int size, int *randomIndices) 
{
    int sum = 0;
    for (int i = 0; i < size; i++) 
    {
        sum += arr[randomIndices[i]];  // Random access
    }
    printf("The cache-hostile sum: %d\n", sum);
}

// Random Indices Array
void generate_randomIndices(int *randomIndices, int size) 
{
    for (int i = 0; i < size; i++) {
        randomIndices[i] = rand() % size;
    }
}

int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        printf("Usage: %s <friendly|hostile>\n", argv[0]);
        return 1;
    }

    // RNG seeding
    srand(time(NULL));

    // Populate array
    for (int i = 0; i < ARRAY_SIZE; i++) 
    {
        array[i] = i;
    }

    // Random idices for hostile access
    int *randomIndices = malloc(ARRAY_SIZE * sizeof(int));
    generate_randomIndices(randomIndices, ARRAY_SIZE);

    // start timing
    clock_t start = clock();

    // Check the argument and perform the respective access pattern
    if (strcmp(argv[1], "friendly") == 0) 
    {
        cache_friendly_access(array, ARRAY_SIZE);
    } 
    else if (strcmp(argv[1], "hostile") == 0) 
    {
        cache_hostile_access(array, ARRAY_SIZE, randomIndices);
    } 
    else 
    {
        printf("Error. Type 'friendly' or 'hostile'.\n");
        free(randomIndices);
        return 1;
    }

    // Finish timing
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("The execution time: %f seconds\n", time_spent);
    free(randomIndices);
    return 0;
}
