#pragma once
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <list>
#include <memory>
#include <iostream>

// BarrelEntry holds the barrel file path and offset
struct BarrelEntry {
    std::string barrel_file;
    size_t offset;
};

class BarrelsReader {
public:
    // Constructor: optionally load manifest file
    BarrelsReader() : max_cache_size(5) {}
    BarrelsReader(const std::string& manifest_file) : max_cache_size(5) {
        std::ifstream fin(manifest_file);
        if (!fin.is_open()) {
            std::cerr << "Cannot open manifest file: " << manifest_file << "\n";
            return;
        }

        int term_id;
        std::string barrel_file;
        size_t offset;
        while (fin >> term_id >> barrel_file >> offset) {
            manifest[term_id] = {barrel_file, offset};
        }
    }

    // Get postings for a term_id
    std::vector<int> get_postings(int term_id);

private:
    // Open barrel file with caching (LRU)
    std::shared_ptr<std::ifstream> open_barrel(const std::string& barrel_file);

    // Barrel manifest
    std::unordered_map<int, BarrelEntry> manifest;

    // LRU cache: filename -> (ifstream, iterator in LRU list)
    std::unordered_map<std::string,
        std::pair<std::shared_ptr<std::ifstream>, std::list<std::string>::iterator>> file_cache;
    std::list<std::string> lru_list;
    size_t max_cache_size;
};
