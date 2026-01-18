#include "stage6_barrels.h"
#include <algorithm>

// ---------------------------
// Open barrel file with LRU
// ---------------------------
std::shared_ptr<std::ifstream> BarrelsReader::open_barrel(const std::string& barrel_file) {
    auto it = file_cache.find(barrel_file);

    if (it != file_cache.end()) {
        // Move to front (most recently used)
        lru_list.erase(it->second.second);
        lru_list.push_front(barrel_file);
        it->second.second = lru_list.begin();
        return it->second.first;
    }

    // Open new file
    auto fin = std::make_shared<std::ifstream>(barrel_file, std::ios::binary);
    if (!fin->is_open()) {
        std::cerr << "Failed to open barrel file: " << barrel_file << "\n";
        return nullptr;
    }

    // Add to LRU cache
    lru_list.push_front(barrel_file);
    file_cache[barrel_file] = {fin, lru_list.begin()};

    if (file_cache.size() > max_cache_size) {
        // Evict least recently used
        std::string lru_file = lru_list.back();
        lru_list.pop_back();
        file_cache.erase(lru_file);
    }

    return fin;
}

// ---------------------------
// Get postings for term_id
// ---------------------------
std::vector<int> BarrelsReader::get_postings(int term_id) {
    std::vector<int> postings;

    auto it = manifest.find(term_id);
    if (it == manifest.end())
        return postings;

    const BarrelEntry& entry = it->second;
    auto fin = open_barrel(entry.barrel_file);
    if (!fin)
        return postings;

    // Seek to offset
    fin->seekg(entry.offset, std::ios::beg);

    int df;
    (*fin) >> df;
    postings.resize(df);

    for (int i = 0; i < df; ++i) {
        (*fin) >> postings[i];
    }

    return postings;
}