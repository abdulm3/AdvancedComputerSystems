NOTE: Formatted results, screenshots, tables, and more details are included in the PDF version of this report.

Running Instructions:
This assignment is split into 4 program files. DictionaryCodec.h and DictionaryCodec.cpp handle implementing the dictionary encoding and query logic, as well as setting up the dictionary encoding scheme. The encoding utilizes the variable num_threads, to define the number of threads used in encoding. The query operations include VanillaScan, Item Search (SIMD/no SIMD), and Prefix Scan (SIMD/No SIMD). Item Search(SIMD/No SIMD) and Prefix Scan (SIMD/No SIMD). The last two files are testbenches that handle encoding and query tests. test_encoding.cpp handles encoding benchmarks with a varying num_thread value (Testing 1,2,4 and 8 thread encoding), and test_query.cpp measures query speeds for the five query operations.

To run this assignment, download the 4 program files, as well as the given raw column data file (Column.txt, provided by the professor at https://drive.google.com/file/d/195XTg8HWDTILc1JlsGX6_jJ5PUhi9KDG/view?usp=drive_link). To compile the program, use the following two commands:

g++ -std=c++17 -O2 -msse4.2 -mavx2 -march=native -o test_encoding test_encoding.cpp DictionaryCodec.cpp
g++ -std=c++17 -O2 -msse4.2 -mavx2 -march=native -o test_query test_query.cpp DictionaryCodec.cpp

The program testbenches can now be run with ./test_encoding and ./test_query respectively.

To modify the query string, adjust line 61 in test_query.cpp. The default implementation is    [ std::string query = "spin"; ], meaning the search query is the string “spin”.
Grading Criteria:
Software Implementation
Encoding Functionality
Dictionary encoding is handled in DictionaryCodec.cpp and manages the encoding of the given Column.txt file. The encoded file is outputted as EncodedColumn.txt (as handled in test_query.cpp) and displays the dictionary and encoded data column.

Query Operations
There are five essential query operations, vanillaScan, queryItem (non SIMD single scan), queryPrefix (non SIMD prefix scan), simdQueryItem (SIMD single scan), simdQueryPrefix (SIMD prefix scan). The test_query.cpp testbench file handles measuring the timing for all of these operations, given a string query.

Multi-Threading Implementation
Multithreading is used within the Column File encoding to speed up the process. The number of lines to encode is equally divided amongst the number of threads specified. In this case, encoding was measured with 1,2,4, and 8 threads.

SIMD Utilization
As mentioned in the previous section, SIMD accelerates single-query and prefix-query searches. This program includes <immintrin.h> to handle SIMD operations.

Performance and Analysis:
Vanilla Column Scan 
The program uses a vanilla column scan (aka scanning without using the dictionary encoding scheme) as a baseline to compare the effect of encoded queries. 

For the query string “spin”, the vanilla scan took 3510 milliseconds. This value is used in the query section later in the report.
Dictionary Encoding
Dictionary encoding was handled across four different thread counts- those being 1,2 4 and 8. A table and graph showing the thread count speed difference is shown below. (Graph in PDF)

Encoding Time (ms) vs. Thread Count
Thread Count - Encoding Time (ms)
1 - 94226
2 - 80589
4 - 75451
8 - 76140

The graph shows that as thread count increases, the time to encode decreases. This functionality works as expected. It is worth noting that there seems to be a bottleneck or diminishing return after 4 threads. 8 threads do not provide any additional performance benefit and seem to be the same as 4 threads (within the margin of error).
Query
For the query test, the query string “spin” was used to run 5 different operations, vanilla scan, item search (No SIMD), item search (SIMD), query scan (No SIMD), query scan (SIMD). A table and graph showing the time to complete these operations are included below.  (Graph in PDF)

Time to complete Query Operations
Operation - Time to Complete (ms)
Vanilla Scan - 3510
Item Search - 2650
Item Search (SIMD) - 289
Prefix Scan - 3531
Prefix Scan (SIMD) - 489

As expected, dictionary encoding provided far faster results than the vanilla scan. Both item searches (SIMD and No-SIMD), provided much faster execution times than the non-encoded vanilla scan did, especially the SIMD implementation. Looking at the prefix scan, we see an expected rise in execution time, due to the new task of searching for a prefix through the entire data file (rather than stopping at the first find). SIMD once again provided a much faster execution time than the No-SIMD version.

Documentation:
Readme Clarity
Readme.txt provided in the repository.

Experimental Setup and Analysis
Experimental setup and analysis are provided in the “Performance and Analysis” portion of this report.

Conclusion & Analysis
The dictionary codec is based on compressing repeated data (encoding) and using indexing to make querying more efficient. In the context of large datasets, such as the one tested here, many values tend to repeat (e.g., string values of common phrases). A dictionary codec exploits this redundancy by doing two things. One, storing unique values in a dictionary, and two, encoding the original dataset as a list of indices pointing to the corresponding unique value in the dictionary.

Benefits of dictionary codec include spare/query efficiency, scalability to larger datasets, faster prefix and range queries, and parallelization by utilizing multiple threads. It is clear in this assignment that multithreaded encoding can improve performance but can diminish returns after a certain number of threads. Dictionary encoding (combined with SIMD optimizations) can greatly reduce execution times compared to a vanilla/raw scan.

In summary, the philosophy of dictionary codecs focuses on reducing data redundancy, improving memory efficiency, and speeding up data access, making it an excellent choice for large-scale data storage and retrieval, especially when combined with parallel and SIMD processing for further acceleration.

