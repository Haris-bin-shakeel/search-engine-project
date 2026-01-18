#pragma once
#include "stage1_lexicon.h"
#include "stage2_forward_index.h"
#include <unordered_map>
#include <cmath>

class Stage4Ranking {
public:
    Stage4Ranking(const ForwardIndex& fwd, const Lexicon& lex);

    double score(int term_id, int doc_id) const;

    // ✅ Expose IDF map for semantic search
    const std::unordered_map<int,double>& get_idf_map() const { return idf_map; }

    // ✅ Expose average document length if needed
    double get_avg_doc_len() const { return avg_doc_len; }

    // Added for Stage 9 compatibility: Update stats after dynamic indexing
    void update_stats();

private:
    const ForwardIndex& fwd_index;
    const Lexicon& lexicon;
    std::unordered_map<int,double> idf_map;
    double avg_doc_len = 0.0;
    
    // Added for Stage 9 compatibility: Allow non-const access for updates
    friend class DynamicIndexer;
};