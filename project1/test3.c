#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#define BUFFER_SIZE 100000000           // 100 MB buffer
#define NUM_THREADS 8                   // # of concurrent threads
#define OPERATIONS_PER_THREAD 10000000  // # of mem ops / thread
int *buffer;  // Main mem buffer

typedef struct 
{
    int threadID;
    int operations;
} 
threadData_t;

void* memoryAccess(void* arg) 
    {
    threadData_t* data = (threadData_t*)arg;
    int id = data->threadID;
    int operations = data->operations;

    struct timeval start, end;
    long elapsedTime = 0;
    
    for (int i = 0; i < operations; i++) 
    {
        int index = rand() % BUFFER_SIZE; //random read and write access within the buffer
        gettimeofday(&start, NULL); 
        int value = buffer[index];
        buffer[index] = value + id;
        gettimeofday(&end, NULL);
        
        // Access time calculation
        elapsedTime += ((end.tv_sec - start.tv_sec) * 1000000L + end.tv_usec) - start.tv_usec;
    }

    // Total time (ms)
    long* result = malloc(sizeof(long));
    *result = elapsedTime;
    return result;
}

int main() {
    buffer = (int*)malloc(BUFFER_SIZE * sizeof(int)); //buffer optimization
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = i;
    }

    // Thread variables
    long total_time = 0;
    long operations_performed = 0;
    pthread_t threads[NUM_THREADS];
    threadData_t thread_data[NUM_THREADS];
  

    // Create and launch threads
    for (int num_threads = 1; num_threads <= NUM_THREADS; num_threads++) 
    {
        total_time = 0;
        operations_performed = 0;
        printf("\nRun using %d thread(s)\n", num_threads);

        for (int t = 0; t < num_threads; t++) 
        {
            thread_data[t].threadID = t;
            thread_data[t].operations = OPERATIONS_PER_THREAD;

            pthread_create(&threads[t], NULL, memoryAccess, (void*)&thread_data[t]);
        }

        for (int t = 0; t < num_threads; t++) 
        {
            long* elapsedTime;
            pthread_join(threads[t], (void**)&elapsedTime);
            total_time += *elapsedTime;
            operations_performed += thread_data[t].operations;
            free(elapsedTime);
        }

        // Calculate average latency and throughput
        double avgLatency = (double)total_time / operations_performed;
        double throughput = (double)operations_performed / (total_time / 1000000.0);

        printf("Average Latency was %.2f microseconds\n", avgLatency);
        printf("Throughput was %.2f operations/second\n", throughput);
    }
    free(buffer);
    return 0;
}
