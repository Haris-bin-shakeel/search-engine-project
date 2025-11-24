#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <nlohmann/json.hpp> // optional for JSON output

struct LexiconEntry {
    int term_id;
    std::string term;
    int df;              // document frequency
    size_t posting_ptr;  // offset in inverted index
};

class Lexicon {
private:
    std::vector<LexiconEntry> entries;
    std::unordered_map<std::string, int> token_to_id;

public:
    void build(const std::string& dataset_path);
    void save_segmented(const std::string& folder);

    int get_term_id(const std::string& token) const {
        auto it = token_to_id.find(token);
        return (it != token_to_id.end()) ? it->second : -1;
    }

    std::vector<LexiconEntry>& get_entries_mutable() { return entries; }
    const std::vector<LexiconEntry>& get_entries() const { return entries; }
};
