#include "DictionaryCodec.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

// Helper function to load the encoded file into a vector of strings
std::vector<std::string> loadFileToVector(const std::string& filename) {
    std::ifstream inFile(filename);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(inFile, line)) {
        lines.push_back(line);
    }
    return lines;
}

void measureQueryPerformance(DictionaryCodec& codec, const std::string& rawFile, const std::string& encodedFile, const std::string& query) {
    // Vanilla Scan Performance
    auto start = std::chrono::high_resolution_clock::now();
    codec.vanillaScan(rawFile, query);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Vanilla scan took: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;

    // Dictionary Query without SIMD
    start = std::chrono::high_resolution_clock::now();
    codec.queryItem(encodedFile, query);  // Single item search without SIMD
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Dictionary query without SIMD took: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;

    // Prefix Query without SIMD
    start = std::chrono::high_resolution_clock::now();
    codec.queryPrefix(encodedFile, query);  // Prefix scan without SIMD
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Prefix query without SIMD took: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;

    // SIMD-based Query
    std::vector<std::string> encodedColumn = loadFileToVector(encodedFile);
    start = std::chrono::high_resolution_clock::now();
    codec.simdQueryItem(encodedColumn, query);  // Single item search with SIMD
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Dictionary query with SIMD took: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;

    // SIMD-based Prefix Query
    start = std::chrono::high_resolution_clock::now();
    codec.simdQueryPrefix(encodedColumn, query);  // SIMD-based prefix scan
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Prefix query with SIMD took: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
}


int main() {
    DictionaryCodec codec;
    std::string rawFile = "Column.txt";
    std::string encodedFile = "EncodedColumn.txt";  // Ensure this matches your file output in encoding
    std::string query = "spin"; //TO USER: you can adjust the "Query String" here. The default query given is searching for the word "spin"

    std::cout << "Measuring query performance..." << std::endl;
    measureQueryPerformance(codec, rawFile, encodedFile, query);

    return 0;
}
