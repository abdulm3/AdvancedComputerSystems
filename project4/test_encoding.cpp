#include "DictionaryCodec.h"
#include <iostream>
#include <chrono>

void measureEncodingPerformance(DictionaryCodec& codec, const std::string& inputFile, const std::string& outputFile) {
    for (int numThreads : {1, 2, 4, 8}) {
        auto start = std::chrono::high_resolution_clock::now();

        // Use the correct method name
        codec.encodeColumnFile(inputFile, outputFile, numThreads);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "Encoding with " << numThreads << " threads took: " << duration << " ms" << std::endl;
    }
}

int main() {
    DictionaryCodec codec;

    std::string inputFile = "Column.txt";  // Input raw column file
    std::string outputFile = "EncodedColumn.txt";  // Encoded output file

    std::cout << "Measuring encoding performance..." << std::endl;
    measureEncodingPerformance(codec, inputFile, outputFile);

    return 0;
}
