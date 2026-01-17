#pragma once
#include <unordered_map>
#include <vector>
#include "stage2_forward_index.h"

class InvertedIndex {
public:
    void build(const ForwardIndex& fwd);
    const std::unordered_map<int,std::vector<int>>& getIndex() const { return inv_index; }

    // Incremental update
    void add_document(int doc_id, const std::vector<int>& term_ids);

private:
    std::unordered_map<int,std::vector<int>> inv_index;
};
