#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "stage2_forward_index.h"
#include "stage1_lexicon.h"

struct Posting {
    int doc_id;
    int term_freq;
    std::vector<int> positions;
};

class InvertedIndex {
private:
    std::unordered_map<int, std::vector<Posting>> index;

public:
    void build(const ForwardIndex& fwd);
    void save_segmented(const std::string& folder, int segment_size = 1000);
    void save_segmented_with_lexicon(const std::string& folder, Lexicon& lex, int segment_size = 1000);
};
