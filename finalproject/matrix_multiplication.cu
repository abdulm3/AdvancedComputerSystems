#include <iostream>
#include <cuda_runtime.h>
#include <cusparse_v2.h>
#include <cublas_v2.h>
#include <chrono>
#include <cstdlib> // For system()
#include <vector> // For dynamic arrays

// Helper function to check for CUDA errors
void checkCUDAError(cudaError_t err, const char* msg) {
    if (err != cudaSuccess) {
        std::cerr << msg << " Error: " << cudaGetErrorString(err) << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Helper function to check for cuSPARSE errors
void checkCUSPARSEError(cusparseStatus_t err, const char* msg) {
    if (err != CUSPARSE_STATUS_SUCCESS) {
        std::cerr << msg << " Error: ";
        if (err == CUSPARSE_STATUS_NOT_INITIALIZED) {
            std::cerr << "CUSPARSE_STATUS_NOT_INITIALIZED" << std::endl;
        }
        else if (err == CUSPARSE_STATUS_ALLOC_FAILED) {
            std::cerr << "CUSPARSE_STATUS_ALLOC_FAILED" << std::endl;
        }
        else if (err == CUSPARSE_STATUS_INVALID_VALUE) {
            std::cerr << "CUSPARSE_STATUS_INVALID_VALUE" << std::endl;
        }
        else {
            std::cerr << "Unknown error" << std::endl;
        }
        exit(EXIT_FAILURE);
    }
}

void runSparseMultiplication(int N, float density) {
    std::cout << "Running sparse matrix multiplication..." << std::endl;

    // Start power logging
    std::system("start /B nvidia-smi --query-gpu=power.draw --format=csv -l 1 > power_log.txt");

    // Initialize cuSPARSE
    cusparseHandle_t handle;
    checkCUSPARSEError(cusparseCreate(&handle), "Failed to create cuSPARSE handle");

    // Use vectors to dynamically allocate space for sparse matrix values
    std::vector<int> h_rowOffsets(N + 1, 0);
    std::vector<int> h_colIndices;
    std::vector<float> h_values;

    // Populate sparse matrix with random values
    srand(0);
    int nnz = 0;
    for (int i = 0; i < N; ++i) {
        h_rowOffsets[i] = nnz; // Start of row
        for (int j = 0; j < N; ++j) {
            if ((rand() / static_cast<float>(RAND_MAX)) < density) {
                h_values.push_back(static_cast<float>(rand()) / RAND_MAX); // Add value
                h_colIndices.push_back(j); // Add column index
                ++nnz; // Increment nnz
            }
        }
    }
    h_rowOffsets[N] = nnz; // End of last row

    // Allocate device memory for CSR format
    float* d_values;
    int* d_rowOffsets;
    int* d_colIndices;
    float* d_denseB, * d_denseC;

    checkCUDAError(cudaMalloc(&d_values, nnz * sizeof(float)), "Failed to allocate device memory for values");
    checkCUDAError(cudaMalloc(&d_rowOffsets, (N + 1) * sizeof(int)), "Failed to allocate device memory for row offsets");
    checkCUDAError(cudaMalloc(&d_colIndices, nnz * sizeof(int)), "Failed to allocate device memory for column indices");
    checkCUDAError(cudaMalloc(&d_denseB, N * N * sizeof(float)), "Failed to allocate device memory for B");
    checkCUDAError(cudaMalloc(&d_denseC, N * N * sizeof(float)), "Failed to allocate device memory for C");

    // Copy CSR data to device
    checkCUDAError(cudaMemcpy(d_rowOffsets, h_rowOffsets.data(), (N + 1) * sizeof(int), cudaMemcpyHostToDevice), "Failed to copy row offsets to device");
    checkCUDAError(cudaMemcpy(d_colIndices, h_colIndices.data(), nnz * sizeof(int), cudaMemcpyHostToDevice), "Failed to copy column indices to device");
    checkCUDAError(cudaMemcpy(d_values, h_values.data(), nnz * sizeof(float), cudaMemcpyHostToDevice), "Failed to copy values to device");

    // Run sparse matrix multiplication using cuSPARSE SpMM
    float alpha = 1.0f, beta = 0.0f;
    cusparseSpMatDescr_t spMatA;
    cusparseDnMatDescr_t dnMatB, dnMatC;
    void* dBuffer = nullptr;
    size_t bufferSize = 0;

    checkCUSPARSEError(cusparseCreateCsr(&spMatA, N, N, nnz, d_rowOffsets, d_colIndices, d_values,
        CUSPARSE_INDEX_32I, CUSPARSE_INDEX_32I, CUSPARSE_INDEX_BASE_ZERO, CUDA_R_32F),
        "Failed to create sparse matrix descriptor");

    checkCUSPARSEError(cusparseCreateDnMat(&dnMatB, N, N, N, d_denseB, CUDA_R_32F, CUSPARSE_ORDER_ROW),
        "Failed to create dense matrix descriptor B");
    checkCUSPARSEError(cusparseCreateDnMat(&dnMatC, N, N, N, d_denseC, CUDA_R_32F, CUSPARSE_ORDER_ROW),
        "Failed to create dense matrix descriptor C");

    checkCUSPARSEError(cusparseSpMM_bufferSize(handle, CUSPARSE_OPERATION_NON_TRANSPOSE, CUSPARSE_OPERATION_NON_TRANSPOSE,
        &alpha, spMatA, dnMatB, &beta, dnMatC, CUDA_R_32F, CUSPARSE_SPMM_ALG_DEFAULT, &bufferSize),
        "Failed to calculate buffer size for SpMM");

    checkCUDAError(cudaMalloc(&dBuffer, bufferSize), "Failed to allocate buffer for SpMM");

    // Measure execution time
    auto start = std::chrono::high_resolution_clock::now();

    checkCUSPARSEError(cusparseSpMM(handle, CUSPARSE_OPERATION_NON_TRANSPOSE, CUSPARSE_OPERATION_NON_TRANSPOSE,
        &alpha, spMatA, dnMatB, &beta, dnMatC, CUDA_R_32F, CUSPARSE_SPMM_ALG_DEFAULT, dBuffer),
        "Failed to perform SpMM");

    checkCUDAError(cudaDeviceSynchronize(), "CUDA synchronization failed");

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Sparse matrix multiplication completed in " << elapsed.count() << " ms" << std::endl;

    // Stop power logging
    std::system("taskkill /IM nvidia-smi.exe /F");

    // Cleanup
    cusparseDestroySpMat(spMatA);
    cusparseDestroyDnMat(dnMatB);
    cusparseDestroyDnMat(dnMatC);
    cudaFree(dBuffer);
    cusparseDestroy(handle);
    cudaFree(d_denseB);
    cudaFree(d_denseC);
    cudaFree(d_rowOffsets);
    cudaFree(d_colIndices);
    cudaFree(d_values);
}

int main() {
    const int N = 16384;    // Matrix size
    const float density = 0.9f; // 0.1f = 10% non-zero elements

    runSparseMultiplication(N, density);

    return 0;
}
