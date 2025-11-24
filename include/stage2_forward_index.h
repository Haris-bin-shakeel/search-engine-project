#pragma once
#include "stage1_lexicon.h"
#include <vector>
#include <string>
#include <unordered_map>

struct ForwardDoc {
    int doc_id;
    std::vector<int> term_ids;
    std::vector<int> term_freqs;
    std::vector<int> positions;
    int length;
};

class ForwardIndex {
private:
    std::vector<ForwardDoc> documents;

public:
    void build(const std::string& dataset_path, const Lexicon& lex);
    void save_segmented(const std::string& folder, int segment_size=1000);
    const std::vector<ForwardDoc>& get_documents() const { return documents; }
};
