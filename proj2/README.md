The project file, proj2.c can be compiled with the command “gcc -mavx2 -o proj2.o proj2.c” and run with the command “./proj2.o” in the terminal. 

The program will ask for user-input for fields regarding row#/col# for both matrices, sparsity (0-1, with 0 being fully dense, 1 being all zeros), multithreading (0-no/1-yes), thread count, use of SIMD optimization (0-no/1-yes), the use of blocking cache miss minimization (0-no/1-yes), and whether or not to print the matrices (0-no/1-yes). 

In the case where the user does not want 2 matrices with the same dimensions/sparsity (such as the Dense-Sparse readings), the user should follow the instructions on line 433 in the program and manually set the two matrices before running. 

Note that for the following tests, the 10,000 matrix size was down-scaled to 5,000 in order to run within a respectable time frame. This policy was approved by the professor.

The data below was gathered on a ThinkPad-T14s Gen 1 Laptop containing an AMD Ryzen 5 Pro 4650U Processor. A set of all general runtimes can be found in bulkdata.txt

Gathered data is reported in the PDF attached (Abdul Muizz - Project 2 - AdvancedComputerSystems.pdf). The graphs within the PDF were generated in this spreadsheet: https://docs.google.com/spreadsheets/d/1BiId6bl0j7WkxflLQXvMY7MJ8QMs5n6QPCHETDuyKFw/edit?usp=sharing
