#include "DictionaryCodec.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <immintrin.h>
#include <iostream>
#include <string>
#include <cstring>
#include <bitset>

void DictionaryCodec::encodeColumnFile(const std::string& inputFile, const std::string& outputFile, int numThreads) {
    std::ifstream inFile(inputFile);
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open input file: " << inputFile << std::endl;
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(inFile, line)) {
        lines.push_back(line);
    }
    inFile.close();
    std::cout << "Loaded " << lines.size() << " lines from input file." << std::endl;

    // Initialize local dictionaries for multi-threading
    std::vector<std::unordered_map<std::string, size_t>> localDictionaries(numThreads);
    std::vector<std::thread> threads;

    try {
        for (int t = 0; t < numThreads; ++t) {
            threads.emplace_back([&, t]() {
                size_t start = t * (lines.size() / numThreads);
                size_t end = (t == numThreads - 1) ? lines.size() : (t + 1) * (lines.size() / numThreads);

                std::cout << "Thread " << t << " processing lines " << start << " to " << end << std::endl;

                for (size_t i = start; i < end; ++i) {
                    localDictionaries[t][lines[i]] = localDictionaries[t].size();
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception during encoding: " << e.what() << std::endl;
        return;
    }

    // Merge dictionaries
    try {
        mergeLocalDictionaries(localDictionaries);
    } catch (const std::exception& e) {
        std::cerr << "Exception during dictionary merge: " << e.what() << std::endl;
        return;
    }

    std::cout << "Encoding complete. Writing to file: " << outputFile << std::endl;

    // Write dictionary and encoded column
    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file: " << outputFile << std::endl;
        return;
    }

    for (const auto& pair : dictionary) {
        outFile << pair.first << " " << pair.second << "\n";
    }

    for (const auto& line : lines) {
        outFile << dictionary[line] << "\n";
    }
    outFile.close();
    std::cout << "Encoded file written successfully." << std::endl;
}

void DictionaryCodec::buildLocalDictionaries(const std::vector<std::string>& lines, 
                                              std::vector<std::unordered_map<std::string, size_t>>& localDictionaries) {
    for (size_t i = 0; i < lines.size(); ++i) {
        size_t threadId = i % localDictionaries.size();
        auto& localDict = localDictionaries[threadId];
        if (localDict.find(lines[i]) == localDict.end()) {
            localDict[lines[i]] = localDict.size();
        }
    }
}

void DictionaryCodec::mergeLocalDictionaries(const std::vector<std::unordered_map<std::string, size_t>>& localDictionaries) {
    for (const auto& localDict : localDictionaries) {
        for (const auto& [key, value] : localDict) {
            std::unique_lock lock(dictionaryMutex);
            if (dictionary.find(key) == dictionary.end()) {
                dictionary[key] = dictionary.size();
            }
        }
    }
}

std::vector<size_t> DictionaryCodec::queryItem(const std::string& encodedFile, const std::string& item) {
    std::ifstream inFile(encodedFile);
    std::string line;
    std::vector<size_t> indices;

    size_t index = 0;
    while (std::getline(inFile, line)) {
        if (line == item) {
            indices.push_back(index);
        }
        ++index;
    }

    inFile.close();
    return indices;
}


std::vector<size_t> DictionaryCodec::simdQueryItem(const std::vector<std::string>& column, const std::string& item) {
    std::vector<size_t> indices;
    size_t itemLength = item.length();

    // Loop through the column data in chunks
    for (size_t i = 0; i < column.size(); i += 8) {
        // Process multiple strings (8 strings at a time)
        for (size_t j = 0; j < 8 && i + j < column.size(); ++j) {
            if (column[i + j].size() == itemLength && 
                std::memcmp(column[i + j].c_str(), item.c_str(), itemLength) == 0) {
                indices.push_back(i + j);
            }
        }
    }

    return indices;
}

std::vector<std::pair<std::string, std::vector<size_t>>> DictionaryCodec::queryPrefix(const std::string& encodedFile, const std::string& prefix) {
    std::ifstream inFile(encodedFile);
    std::string line;
    std::vector<std::pair<std::string, std::vector<size_t>>> result;

    size_t index = 0;
    while (std::getline(inFile, line)) {
        if (line.find(prefix) == 0) {
            result.push_back({line, {index}});
        }
        ++index;
    }

    inFile.close();
    return result;
}

std::vector<size_t> DictionaryCodec::vanillaScan(const std::string& rawFile, const std::string& query) {
    std::ifstream inFile(rawFile);
    std::vector<size_t> indices;
    std::string line;

    size_t index = 0;
    while (std::getline(inFile, line)) {
        if (line == query) {
            indices.push_back(index);
        }
        ++index;
    }

    inFile.close();
    return indices;
}


std::vector<std::pair<std::string, std::vector<size_t>>> DictionaryCodec::simdQueryPrefix(const std::vector<std::string>& column, const std::string& prefix) {
    std::vector<std::pair<std::string, std::vector<size_t>>> result;
    size_t prefixLength = prefix.length();

    // Loop through the column data in chunks
    for (size_t i = 0; i < column.size(); i += 8) {
        // Process multiple strings (8 strings at a time)
        for (size_t j = 0; j < 8 && i + j < column.size(); ++j) {
            const std::string& currentString = column[i + j];

            // Check if the string has at least the prefix length
            if (currentString.size() >= prefixLength && 
                std::memcmp(currentString.c_str(), prefix.c_str(), prefixLength) == 0) {
                // Prefix matches, add to result
                result.push_back({currentString, {i + j}});
            }
        }
    }

    return result;
}

