#include "stage2_forward_index.h"
#include <sstream>

void ForwardIndex::build_from_docs(const std::vector<std::string>& docs, const Lexicon& lex) {
    fwd_index.clear();
    for (int doc_id = 0; doc_id < (int)docs.size(); ++doc_id) {
        // Industry standard: Use centralized tokenization with stopword filtering
        std::vector<std::string> tokens = Lexicon::tokenize_and_filter(docs[doc_id]);
        for (const auto& token : tokens) {
            int term_id = lex.get_term_id(token);
            if (term_id >= 0) {
                fwd_index[doc_id].push_back(term_id);
            }
        }
    }
}

// Added for Stage 9 compatibility: Incremental document addition
void ForwardIndex::add_document(int doc_id, const std::vector<int>& term_ids) {
    fwd_index[doc_id] = term_ids;
}