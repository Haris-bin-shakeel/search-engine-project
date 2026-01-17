#include "stage2_forward_index.h"
#include <sstream>

void ForwardIndex::build_from_docs(const std::vector<std::string>& docs, const Lexicon& lex) {
    fwd_index.clear();
    for (int doc_id = 0; doc_id < (int)docs.size(); ++doc_id) {
        std::istringstream iss(docs[doc_id]);
        std::string token;
        while (iss >> token) {
            int term_id = lex.get_term_id(token);
            if (term_id >= 0) {
                fwd_index[doc_id].push_back(term_id);
            }
        }
    }
}
