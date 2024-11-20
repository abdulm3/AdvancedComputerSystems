This assignment is split into 4 program files. DictionaryCodec.h and DictionaryCodec.cpp handle implementing the dictionary encoding and query logic, as well as setting up the dictionary encoding scheme. The encoding utilizes the variable num_threads, to define the number the threads used in encoding. The query operations include VanillaScan, Item Search (SIMD/no SIMD), and Prefix Scan (SIMD/No SIMD). Item Search(SIMD/No SIMD) and Prefix Scan (SIMD/No SIMD). The last two files are testbenches to handle encoding and query tests. test_encoding.cpp handles encoding benchmarks with a varying num_thread value (Testing 1,2,4 and 8 thread encoding), and test_query.cpp measures query speeds for 5 the five query operations.

To run this assignment, download the 4 program files, as well as the given raw column data file (Column.txt, provided by the professor at https://drive.google.com/file/d/195XTg8HWDTILc1JlsGX6_jJ5PUhi9KDG/view?usp=drive_link). To compile the program, use the following two commands:

g++ -std=c++17 -O2 -msse4.2 -mavx2 -march=native -o test_encoding test_encoding.cpp DictionaryCodec.cpp
g++ -std=c++17 -O2 -msse4.2 -mavx2 -march=native -o test_query test_query.cpp DictionaryCodec.cpp

The program testbenches can now be run with ./test_encoding and ./test_query respectively.

To modify the query string, adjust line 61 in test_query.cpp. The default implementation is    [ std::string query = "spin"; ], meaning the search query is the string “spin”.
