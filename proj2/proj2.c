#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <immintrin.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
//Abdul Muizz - Project2, ECSE6320 Advanced Computer Systems

//main matrix structure
struct matrix{ //Matrix Struct
    int rows; //Row count
    int cols; //Column count
    float **values; //Matrix pointer
};

//function to calculate elapsed time
double measure_time(struct timespec start, struct timespec finish){ //returns computation time (seconds)
    double starting = start.tv_sec + start.tv_nsec / 1e9;
    double finishing = finish.tv_sec + finish.tv_nsec / 1e9;
    
    return finishing-starting;
};

//function to free the memory space used by a matrix
void freeMatrix(struct matrix mat) {
    for (int i = 0; i < mat.rows; i++) {
        free(mat.values[i]);
    }
    free(mat.values);
}

//function to print a matrix (for debugging purposes)
void matrix_print(struct matrix m){
    for (int idx = 0; idx<m.rows; idx++)
        for (int idx2 = 0; idx2<m.cols; idx2++){
            printf("%f ", m.values[idx][idx2]);
        }
        printf("\n");  //newline
}

//this function can be used to compare the dimensions of two matrices, and each element within them for equality
bool compareMatrix(struct matrix a, struct matrix b, float tolerance){
    if (a.rows != b.rows || a.cols != b.cols){
        printf("Matrix dimensions don't match!\n");
        return false;
    }
    int idx, idx2;
    for (idx=0; idx <a.rows; idx++){
        for(idx2=0; idx2<a.cols; idx2++){
            if(fabs(a.values[idx][idx2]-b.values[idx][idx2])>tolerance){
                printf("Element %d %d don't match. %.2f vs %.2f\n",idx, idx2, a.values[idx][idx2],b.values[idx][idx2]);
            }
        }
    }
    return true;
}

//struct to setup multithreading for matrix multiplication
struct thread_function {//Struct to thread a function
    int rowCount;
    struct matrix *a;
    struct matrix *b;
    struct matrix *res;
    int firstRow;
    int finalRow;
};

//allocate memory space for a matrix
struct matrix allocateMat(int rows, int cols){
    struct matrix allocMat;
    allocMat.values = (float **)malloc(rows * sizeof(float *));
    for (int idx = 0; idx < rows; idx++){
        allocMat.values[idx] = (float *)malloc(cols * sizeof(float));
    }
    allocMat.rows = rows;
    allocMat.cols = cols;
    return allocMat;
}

//generate a matrix based on dimension, and sparsity. Printable if needed for debugging
struct matrix generateMatrix (int rows, int cols, float sparsity, bool printing) {
    struct matrix genMat;
    genMat.values = (float **)malloc(rows * sizeof(float *));
    genMat.rows = rows,
    genMat.cols = cols;
    for (int idx = 0; idx < rows; idx++){
        genMat.values[idx] = (float *)malloc(cols * sizeof(float));
    }

    for (int idx = 0; idx < rows; idx++){
        for (int idx2 = 0; idx2 < cols; idx2++) {
            float randomval = (float)rand() / RAND_MAX;
            if (randomval >= sparsity) {
                genMat.values[idx][idx2] = rand()%100 +1;
            }
            else{
                genMat.values[idx][idx2] = 0;
            }
        }
    }
    if (printing) {
        printf("Matrix dimension %d x %d. Sparisty is %.2f:\n",rows, cols, sparsity);
        matrix_print(genMat);
    }
    return genMat;
}

//basic matrix multiplication without any additional optimizations
struct matrix unoptimized_multiply(struct matrix a, struct matrix b){
    if (a.cols == b.rows){
        struct matrix res;
        res = allocateMat (a.rows, b.cols);
        for (int idx = 0; idx < res.rows; idx++){
            for (int idx2 = 0; idx2 < res.cols; idx2++) {
                res.values[idx][idx2] = 0;
                for (int idx3 = 0; idx3 < a.cols; idx3++){
                    res.values[idx][idx2] += a.values[idx][idx3]*b.values[idx3][idx2];
                }
            }
        }
        return res;   
    }
    else{
        printf("Matrix dimensions not compatible for multiplication\n");
        exit(1);
    }
}

//SIMD optimizations without threading or blocking
struct matrix multiplySIMD(struct matrix a, struct matrix b) {
    // Check if multiplication is possible
    if (a.cols != b.rows) {
        printf("matrix dimensions can't be used for multiplication.\n");
        exit(1);
    }

    // Allocate result matrix (a.rows x b.cols)
    struct matrix result = allocateMat(a.rows, b.cols);

    for (int i = 0; i < a.rows; i++) {
        for (int j = 0; j < b.cols; j++) {
            __m256 sum = _mm256_setzero_ps();
            for (int k = 0; k < a.cols; k += 8) {
                __m256 aVals = _mm256_loadu_ps(&a.values[i][k]);
                __m256 bVals = _mm256_loadu_ps(&b.values[k][j]);
                __m256 mulVals = _mm256_mul_ps(aVals, bVals);
                sum = _mm256_add_ps(sum, mulVals);
            }
            float resultVals[8];
            _mm256_storeu_ps(resultVals, sum);
            result.values[i][j] = resultVals[0] + resultVals[1] + resultVals[2] + resultVals[3] + resultVals[4] + resultVals[5] + resultVals[6] + resultVals[7];
        }
    }

    return result;
}

//multiply with blocking (minimize cache miss optimization)
struct matrix multiplyBlock(struct matrix a, struct matrix b){
    int BLOCK_SIZE = 64;
    if(a.cols != b.rows){
        printf("Incompatible dimensions.\n");
        exit(1);
    }
    struct matrix result = allocateMat(a.rows, b.cols);
    for(int i = 0; i<result.rows; i++){
        for(int j=0; j<result.cols; j++){
            result.values[i][j] = 0;
        }
    }
    for(int i = 0;i<a.rows;i+= BLOCK_SIZE){
        for(int j=0;j<b.cols;j+=BLOCK_SIZE){
            for(int k=0;k<a.cols;k+=BLOCK_SIZE){
                for(int blocki=i; blocki<i+BLOCK_SIZE && blocki<a.rows;blocki++){
                    for(int blockj=j; blockj<j+BLOCK_SIZE && blockj<b.cols; blockj++){
                        for(int blockk=k; blockk<k+BLOCK_SIZE && blockk <a.cols; blockk++){
                            if(a.values[blocki][blockk] != 0 && b.values[blockk][blockj] != 0){
                                result.values[blocki][blockj] += a.values[blocki][blockk] * b.values[blockk][blockj];
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}
//multiplication with added threaded optimization
void *thread_multiply(void *inpt){
    struct thread_function *cinpt = (struct thread_function *)inpt;
    struct matrix *res = cinpt->res;
    struct matrix *a = cinpt->a;
    struct matrix *b = cinpt->b;
    int firstRow = cinpt->firstRow;
    int finalRow = cinpt->finalRow;
    for(int i = firstRow; i<finalRow; i++){
        for(int j = 0; j < b->cols;j++){
            res->values[i][j] = 0;
            for (int k=0; k<a->cols;k++){
                res->values[i][j] += a->values[i][k] * b->values[k][j];
            }
        }
    }
    pthread_exit(NULL);
}

//matrix multiplication using SIMD functions
//Pass in 1 thread to use non-multithreaded version (Block+SIMD optimizations)
void *thread_multiplySIMD(void *inpt) {
    struct thread_function *values = (struct thread_function *)inpt;
    struct matrix *res = values->res;
    struct matrix *a = values->a;
    struct matrix *b = values->b;
    
    int firstRow = values->firstRow;
    int finalRow = values->finalRow;

    for (int i = firstRow; i < finalRow; i++) {
        for (int j = 0; j < b->cols; j++) {
            __m256 sum = _mm256_setzero_ps(); 
            int k;
            for (k = 0; k <= a->cols - 8; k += 8) {
                __m256 a_vals = _mm256_loadu_ps(&a->values[i][k]);
                __m256 b_vals = _mm256_loadu_ps(&b->values[k][j]);
                __m256 mul_vals = _mm256_mul_ps(a_vals, b_vals);
                sum = _mm256_add_ps(sum, mul_vals);
            }

            float results[8];
            _mm256_storeu_ps(results, sum);
            res->values[i][j] = results[0] + results[1] + results[2] + results[3] + results[4] + results[5] + results[6] + results[7];

            for(k=0; k < a->cols; k++) {
                res->values[i][j] += a->values[i][k] * b->values[k][j];
            }
        }
    }
    return NULL;
}

//matrix multiplication using blocking to help minimize cache misses
void *thread_multiplyBLOCK(void *inpt) {
    int BLOCK_SIZE = 64;
    struct thread_function *inpts = (struct thread_function *)inpt;
    struct matrix *res = inpts->res;
    struct matrix *a = inpts->a;
    struct matrix *b = inpts->b;
    
    int firstRow = inpts->firstRow;
    int finalRow = inpts->finalRow;

    // Block mult
    for (int blocki = firstRow; blocki < finalRow; blocki += BLOCK_SIZE) {
        for (int blockj = 0; blockj < b->cols; blockj += BLOCK_SIZE) {
            for (int blockk = 0; blockk < a->cols; blockk += BLOCK_SIZE) {

                for (int i = blocki; i < blocki + BLOCK_SIZE && i < finalRow; i++) {
                    for (int j = blockj; j < blockj + BLOCK_SIZE && j < b->cols; j++) {

                        float sum = 0;
                        for (int k = blockk; k < blockk + BLOCK_SIZE && k < a->cols; k++) {
                            if (a->values[i][k] != 0) { 
                                sum += a->values[i][k] * b->values[k][j];
                            }
                        }
                        res->values[i][j] += sum;
                    }
                }
            }
        }
    }
    return NULL;
}

//this function combines blocking and SIMD optimizations
void *thread_multiplyBlockSIMD(void *inpt) {
    int BLOCK_SIZE = 64;
    struct thread_function *inpts = (struct thread_function *)inpt;
    struct matrix *res = inpts->res;
    struct matrix *a = inpts->a;
    struct matrix *b = inpts->b;
    
    int firstRow = inpts->firstRow;
    int finalRow = inpts->finalRow;

    for (int blocki = firstRow; blocki < finalRow; blocki += BLOCK_SIZE) {
        for (int blockj = 0; blockj < b->cols; blockj += BLOCK_SIZE) {
            for (int blockk = 0; blockk < a->cols; blockk += BLOCK_SIZE) {

                for (int i = blocki; i < blocki + BLOCK_SIZE && i < finalRow; i++) {
                    for (int j = blockj; j < blockj + BLOCK_SIZE && j < b->cols; j++) {
                        __m256 resultVec = _mm256_setzero_ps();  // SIMD result vect

                        // Use SIMD mult (8 elements at a time)
                        for (int k = blockk; k < blockk + BLOCK_SIZE && k < a->cols; k += 8) {  

                            if (a->values[i][k] != 0) {
                                // Load 8 elements
                                __m256 aVec = _mm256_loadu_ps(&a->values[i][k]);
                                __m256 bVec = _mm256_loadu_ps(&b->values[k][j]);
                                // "fused" multiply-add
                                __m256 mulVec = _mm256_mul_ps(aVec, bVec);
                                resultVec = _mm256_add_ps(resultVec, mulVec);
                            }
                        }
                        float tempElement[8];
                        _mm256_storeu_ps(tempElement, resultVec);
                        for (int z = 0; z < 8; z++) {
                            res->values[i][j] += tempElement[z];
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

struct matrix multithread_multiply(struct matrix a, struct matrix b, int threadCount, int simd, int block) {
    // Check if multiplication is possible
    if (a.cols != b.rows) {
        printf("matrix dimensions can't be used for multiplication.\n");
        exit(1);
    }
    struct matrix res = allocateMat(a.rows, b.cols);
    int threadRows = a.rows / threadCount;
    int rowsRemain = a.rows % threadCount;

    pthread_t threads[threadCount];
    struct thread_function inpt[threadCount];

    // Create threads to compute each row of the result matrix
    for (int i = 0; i < threadCount; i++) {
        int firstRow = i * threadRows;
        int finalRow = firstRow + threadRows;

        if (i == threadCount - 1) {  // Last thread handles remaining rows
            finalRow += rowsRemain;
        }

        inpt[i].a = &a;
        inpt[i].b = &b;
        inpt[i].res = &res;
        inpt[i].firstRow = firstRow;
        inpt[i].finalRow = finalRow;

        if (simd && block) {pthread_create(&threads[i], NULL, thread_multiplyBlockSIMD, (void *)&inpt[i]);}
        if (block && !simd) {pthread_create(&threads[i], NULL, thread_multiplyBLOCK, (void *)&inpt[i]);}
        if (simd && !block) {pthread_create(&threads[i], NULL, thread_multiplySIMD, (void *)&inpt[i]);}
        
        else{pthread_create(&threads[i], NULL, thread_multiply, (void *)&inpt[i]);}
    }

    // Wait for all threads to finish
    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], NULL);
    }

    return res;
}




int main() {
    int rows, cols;
    int opt, multithread, simd, blocking, threadCount, print;
    float sparsity;
    srand(time(NULL));  // Seed random number generator

    //THIS BLOCK of commented out code can be used to generate the bulkdata.txt file, in place of the rest of the main code.
    /*
    struct timespec start, finish;
    //vary size and sparsity
    int n;
    float s;
    int kthreads;
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        for(int k = 0; k < 2; k++){
            for(int l = 0; l<2; l++){
                for (int m=0; m<2; m++){
                    switch (i) {
                    case 0: n = 1000; break;
                    case 1: n = 2000; break;
                    case 2: n = 5000; break;
                    }
                    switch (j) {
                    case 0: s = 0.01; break;
                    case 1: s = 0.05; break;
                    case 2: s = 0.1; break;
                    }
                    kthreads =1;
                    if(k==1){
                        kthreads=8;
                    }
                    //printf("Multiplying with... n: %d, sparsity: %.2f\n", n, s);
                    struct matrix a = generateMatrix(n, n, s, 0);
                    struct matrix b = generateMatrix(n, n, s, 0);
                    struct matrix resultsMat;
                    clock_gettime(CLOCK_MONOTONIC, &start);  // Start time
                    resultsMat = multithread_multiply(a, b, kthreads, l, m); //a, b, threadcount, simd(0/1), block(0/1)
                    clock_gettime(CLOCK_MONOTONIC, &finish);  // End time
                    printf("Matrix mult. took %.12f seconds for %d dim, %.2f sparsity, %d threads, %d SIMD, %d block\n", measure_time(start, finish), n,s,kthreads,l,m);
                }
            }
        }
      }
    }   
    */


    //User inputs
    printf("Enter # of rows: "); scanf("%d", &rows);
    printf("Enter # of cols: "); scanf("%d", &cols);
    printf("Enter sparsity 0-1 (dense-zeroes): "); scanf("%f", &sparsity);
    printf("Multithreading? (0:no, 1:yes)\n"); scanf("%d", &multithread);
    if (multithread == 1){
        printf("How many threads? (1-#rows)\n"); scanf("%d", &threadCount);
    }
    else{
        threadCount = 1;
    }
    printf("Use SIMD? (0:no, 1:yes)\n");scanf("%d", &simd);
    printf("Use Blocking (Cache Miss minimization) (0:no, 1:yes)\n"); scanf("%d",&blocking);
    printf("Print? (0:no, 1:yes)\n");scanf("%d", &print);
    //Generate matrices
    printf("# of rows: %d, # of cols %d\n", rows,cols); //matrix1 columns should equal matrix2 rows
    struct matrix matrix1 = generateMatrix(rows, cols, sparsity, print);
    struct matrix matrix2 = generateMatrix(rows, cols, sparsity, print);

    //USER EDIT: If you wish for matrix1 and matrix2 to have different values (such as for the dense-sparse experiment),
    //comment out the two structs above and manually set them in the two lines below (inputs are: #rows, #columns, sparsity(0-1), print(0/1)) 0 sparsity is dense-1 is all zeroes, 0 is no print and vice-versa
    
    //struct matrix matrix1 = generateMatrix(rows, cols, sparsity, print);
    //struct matrix matrix2 = generateMatrix(rows, cols, sparsity, print);

    //generate result matrices
    struct matrix result1;
    struct matrix result2;
    struct matrix result3;
    struct matrix result4;
    struct matrix result5;
    //setup timer
    struct timespec start, finish;
    
    if(!multithread && !simd && !blocking){
      clock_gettime(CLOCK_MONOTONIC, &start);  //Start timer

      // No optimization test
      result1 = unoptimized_multiply(matrix1, matrix2);

      clock_gettime(CLOCK_MONOTONIC, &finish);  //End timer
    
      double elapsed_time = measure_time(start, finish);
      printf("Matrix mult. took %.12f seconds for %dx%d dim, %.2f sparsity, %d threads, %d SIMD, %d block\n", elapsed_time, rows,cols, sparsity,threadCount,simd,blocking);

      // Print the result1 matrix
      if (print) {
        printf("Result matrix:\n");
        matrix_print(result1);
      }
      freeMatrix(result1);
    }

    if (multithread){
      clock_gettime(CLOCK_MONOTONIC, &start);  //Start timer

      //any multithreading test  
      result2 = multithread_multiply(matrix1, matrix2, threadCount, simd, blocking);

      clock_gettime(CLOCK_MONOTONIC, &finish);  //End timer

      double elapsed_time = measure_time(start, finish);
      printf("Matrix mult. took %.12f seconds for %dx%d dim, %.2f sparsity, %d threads, %d SIMD, %d block\n", elapsed_time, rows,cols, sparsity,threadCount,simd,blocking);

      if (print) {
        printf("Result matrix:\n");
        matrix_print(result2);
      }
      freeMatrix(result2);
    }

    if (simd && !multithread && !blocking){
      clock_gettime(CLOCK_MONOTONIC, &start);  //Start timer

      //only SIMD test 
      result3 = multiplySIMD(matrix1, matrix2);

      clock_gettime(CLOCK_MONOTONIC, &finish);  //End timer

      double elapsed_time = measure_time(start, finish);
      printf("Matrix mult. took %.12f seconds for %dx%d dim, %.2f sparsity, %d threads, %d SIMD, %d block\n", elapsed_time, rows,cols, sparsity,threadCount,simd,blocking);

      if (print) {
        printf("Result matrix:\n");
        matrix_print(result3);
      }
      freeMatrix(result3);
    }

    if (blocking && !simd && !multithread){
      clock_gettime(CLOCK_MONOTONIC, &start);  //Start timer
      //only blocking test
      result4 = multiplyBlock(matrix1, matrix2);

      clock_gettime(CLOCK_MONOTONIC, &finish);  //End timer

      double elapsed_time = measure_time(start, finish);
      printf("Matrix mult. took %.12f seconds for %dx%d dim, %.2f sparsity, %d threads, %d SIMD, %d block\n", elapsed_time, rows,cols, sparsity,threadCount,simd,blocking);

      if (print) {
        printf("Result matrix:\n");
        matrix_print(result4);
      }
      freeMatrix(result4);
    }

    if (blocking && simd && !multithread){
      clock_gettime(CLOCK_MONOTONIC, &start);  //Start timer
      //blocking and simd on 1 thread test
      result5 = multithread_multiply(matrix1, matrix2, 1, simd, 1); //use only 1 thread

      clock_gettime(CLOCK_MONOTONIC, &finish);  //End timer

      double elapsed_time = measure_time(start, finish);
      printf("Matrix mult. took %.12f seconds for %dx%d dim, %.2f sparsity, %d threads, %d SIMD, %d block\n", elapsed_time, rows,cols, sparsity,threadCount,simd,blocking);

      if (print) {
        printf("Result matrix:\n");
        matrix_print(result5);
      }
      freeMatrix(result5);
    }

    // Free all matrices
    freeMatrix(matrix1);
    freeMatrix(matrix2);
    //ender
    
  
    return 0;
}
