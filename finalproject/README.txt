# GPU Sparse Matrix Multiplication and Power Profiling

This program performs **sparse matrix multiplication** using the NVIDIA CUDA framework and **cuSPARSE library** on the GPU. It measures the execution time for the multiplication and logs the GPU's power consumption throughout the process, providing insights into performance and energy efficiency.

---

## **Key Features**
1. **Sparse Matrix Multiplication:**
   - Implements matrix multiplication for sparse matrices in Compressed Sparse Row (CSR) format.
   - Uses NVIDIA's cuSPARSE library for optimized sparse operations.

2. **GPU Power Profiling:**
   - Continuously monitors GPU power consumption using `nvidia-smi`.
   - Logs power usage data to `power_log.txt` for analysis.

3. **Execution Time Measurement:**
   - Accurately times the matrix multiplication computation on the GPU using `std::chrono`.

4. **Customizable Parameters:**
   - **Matrix size (`N`)**: Controls the dimensions of the square matrix.
   - **Density (`density`)**: Defines the sparsity of the matrix (percentage of non-zero elements).

---

## **How It Works**

1. **Matrix Generation:**
   - Generates a random sparse matrix in CSR format with a user-defined density.
   - Allocates memory dynamically using `std::vector`.

2. **CUDA and cuSPARSE Operations:**
   - Allocates GPU memory and transfers the matrix data.
   - Performs sparse matrix multiplication (`SpMM`) on the GPU using cuSPARSE.

3. **Timing:**
   - Captures the time taken for the matrix multiplication on the GPU.

4. **Power Logging:**
   - Starts `nvidia-smi` in the background to log GPU power draw every second into `power_log.txt`.
   - Stops the logging process upon completion.

5. **Output:**
   - Prints the execution time to the console.
   - Logs power consumption in `power_log.txt`.

---

## **Usage**

1. **Prerequisites:**
   - An NVIDIA GPU with CUDA support.
   - NVIDIA CUDA Toolkit installed (including cuSPARSE).
   - `nvidia-smi` available in the system PATH.

2. **Compiling:**
   - Save the program as `matrix_multiplication.cu`.
   - Compile using `nvcc`:
     ```bash
     nvcc matrix_multiplication.cu -o matrix_multiplication -lcusparse
     ```

3. **Running:**
   - Execute the program:
     ```bash
     ./matrix_multiplication
     ```

4. **Customizing Parameters:**
   - Modify the matrix size (`N`) and density (`density`) in the `main()` function:
     ```cpp
     const int N = 16384;    // Matrix size
     const float density = 0.1f; // Sparsity level (10% non-zero elements)
     ```

---

## **Output Files**
- **Console Output:**
  - Displays the execution time in milliseconds:
    ```
    Sparse matrix multiplication completed in 120.45 ms
    ```
- **Power Log (`power_log.txt`):**
  - Contains GPU power draw readings during the multiplication, e.g.:
    ```
    power.draw [W]
    91.16 W
    92.34 W
    ...
    ```

---

## **Example Use Case**

### **Scenario:**
Analyze the power consumption and execution time of multiplying a sparse matrix with:
- Size: `16,384 x 16,384`
- Density: `10% non-zero elements`

### **Expected Output:**
- **Console Output:**
  ```
  Sparse matrix multiplication completed in 300.25 ms
  ```
- **Power Log:**
  A time-series log of GPU power draw in watts.

---