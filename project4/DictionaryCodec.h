#ifndef DICTIONARY_CODEC_H
#define DICTIONARY_CODEC_H

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <shared_mutex>

class DictionaryCodec {
public:
    void encodeColumnFile(const std::string& inputFile, const std::string& outputFile, int numThreads);
    void decodeColumnFile(const std::string& encodedFile, const std::string& decodedFile);
    
    std::vector<size_t> queryItem(const std::string& encodedFile, const std::string& item);
    std::vector<std::pair<std::string, std::vector<size_t>>> queryPrefix(const std::string& encodedFile, const std::string& prefix);

    // Vanilla scan implementation
    std::vector<size_t> vanillaScan(const std::string& rawFile, const std::string& query);
    
    // SIMD and scalar search functions
    std::vector<size_t> simdQueryItem(const std::vector<std::string>& column, const std::string& item);
    std::vector<std::pair<std::string, std::vector<size_t>>> simdQueryPrefix(const std::vector<std::string>& column, const std::string& prefix);

private:
    std::unordered_map<std::string, size_t> dictionary;
    std::shared_mutex dictionaryMutex;

    void buildLocalDictionaries(const std::vector<std::string>& lines, 
                                std::vector<std::unordered_map<std::string, size_t>>& localDictionaries);
    void mergeLocalDictionaries(const std::vector<std::unordered_map<std::string, size_t>>& localDictionaries);

};

#endif
