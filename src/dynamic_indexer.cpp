#include "dynamic_indexer.h"
#include <sstream>

int DynamicIndexer::add_document(const std::string& doc_text) {
    std::istringstream iss(doc_text);
    std::string token;
    std::vector<int> term_ids;

    while (iss >> token) {
        int term_id = lexicon.get_term_id(token);
        if (term_id < 0) {
            // New token
            lexicon.build_from_docs({token});
            term_id = lexicon.get_term_id(token);
        }
        term_ids.push_back(term_id);
    }

    int new_doc_id = static_cast<int>(fwd_index.getIndex().size());
    fwd_index.getIndex()[new_doc_id] = term_ids;

    inv_index.add_document(new_doc_id, term_ids);
    ranking.add_document_terms(term_ids);

    return new_doc_id;
}
