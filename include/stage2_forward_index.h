#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "stage1_lexicon.h"

class ForwardIndex {
public:
    void build_from_docs(const std::vector<std::string>& docs, const Lexicon& lex);
    const std::unordered_map<int,std::vector<int>>& getIndex() const { return fwd_index; }

    // Added for Stage 9 compatibility: Incremental document addition
    void add_document(int doc_id, const std::vector<int>& term_ids);

private:
    std::unordered_map<int,std::vector<int>> fwd_index;
};