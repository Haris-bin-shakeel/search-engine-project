#include "stage5_query_engine.h"
#include "stage7_semantic.h" // include full class

#include <algorithm>
#include <cctype>
#include <sstream>

std::vector<SearchResult> QueryEngine::search(const std::string& query, int top_k) {
    std::vector<SearchResult> results;

    // Industry standard: Tokenize query with stopword filtering (same as indexing)
    std::vector<std::string> tokens = Lexicon::tokenize_and_filter(query);
    std::vector<int> query_term_ids;
    for (const auto& token : tokens) {
        int term_id = lexicon.get_term_id(token);
        if (term_id != -1) query_term_ids.push_back(term_id);
    }

    // Industry standard: Merge static + delta postings at query time
    std::unordered_map<int, double> doc_scores;
    for (int term_id : query_term_ids) {
        // Get postings from static index
        auto static_it = inv_index.getIndex().find(term_id);
        if (static_it != inv_index.getIndex().end()) {
            for (int doc_id : static_it->second) {
                doc_scores[doc_id] += 1.0; // simple frequency-based scoring
            }
        }
        
        // Get postings from delta index (Stage 9)
        if (delta_inv_index) {
            auto delta_it = delta_inv_index->find(term_id);
            if (delta_it != delta_inv_index->end()) {
                for (int doc_id : delta_it->second) {
                    doc_scores[doc_id] += 1.0; // same scoring
                }
            }
        }
    }

    // Convert to vector
    for (auto& [doc_id, score] : doc_scores) {
        SearchResult res;
        res.doc_id = doc_id;
        res.score = score;
        res.snippet = ""; // could fill from ForwardIndex if desired
        results.push_back(res);
    }

    // Apply semantic reranking if available
    if (semantic && fwd_index) semantic->rerank(query, results, lexicon, Stage4Ranking(*fwd_index, lexicon));


    // Sort by score descending
    std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        return a.score > b.score;
    });

    if (results.size() > (size_t)top_k) results.resize(top_k);
    return results;
}