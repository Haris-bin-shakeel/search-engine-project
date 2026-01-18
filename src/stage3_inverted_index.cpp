#include "stage3_inverted_index.h"
#include "stage2_forward_index.h"

void InvertedIndex::build(const ForwardIndex& fwd) {
    inv_index.clear();
    const auto& fwd_idx = fwd.getIndex();
    for (const auto& [doc_id, terms] : fwd_idx) {
        for (int term_id : terms) {
            inv_index[term_id].push_back(doc_id);
        }
    }
}

void InvertedIndex::add_document(int doc_id, const std::vector<int>& term_ids) {
    for (int term_id : term_ids) {
        inv_index[term_id].push_back(doc_id);
    }
}