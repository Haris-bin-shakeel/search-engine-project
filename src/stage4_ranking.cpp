#include "stage4_ranking.h"

Stage4Ranking::Stage4Ranking(const ForwardIndex& fwd, const Lexicon& lex)
    : fwd_index(fwd), lexicon(lex)
{
    // Compute average document length
    size_t total_len = 0;
    for (const auto& [doc_id, terms] : fwd_index.getIndex()) {
        total_len += terms.size();
    }
    avg_doc_len = total_len / static_cast<double>(fwd_index.getIndex().size());

    // Compute IDF for each term
    int N = static_cast<int>(fwd_index.getIndex().size());
    for (const auto& [token, term_id] : lexicon.get_token_to_id()) {
        int df = lexicon.get_df(term_id);
        idf_map[term_id] = std::log((N - df + 0.5) / (df + 0.5) + 1.0);
    }
}

double Stage4Ranking::score(int term_id, int doc_id) const {
    const auto& fwd_idx = fwd_index.getIndex();
    auto it = fwd_idx.find(doc_id);
    if (it == fwd_idx.end()) return 0.0;

    int tf = 0;
    for (int t : it->second) if (t == term_id) ++tf;

    double idf = idf_map.at(term_id);
    double k1 = 1.5, b = 0.75;
    int doc_len = static_cast<int>(it->second.size());

    return idf * tf * (k1 + 1) / (tf + k1 * (1 - b + b * doc_len / avg_doc_len));
}
