#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "stage1_lexicon.h"
#include "stage2_forward_index.h"
#include "stage3_inverted_index.h"
#include "stage4_ranking.h"
#include "stage6_barrels.h"

// Forward declaration for Stage 7
class SemanticEngine;

struct SearchResult {
    int doc_id;
    double score;
    std::string snippet;
};

class QueryEngine {
public:
    // Constructor: only take references to Lexicon and InvertedIndex
    QueryEngine(const Lexicon& lex, const InvertedIndex& inv)
        : lexicon(lex), inv_index(inv) {}

    void attach_forward_index(const ForwardIndex& fwd) { fwd_index = &fwd; }
    void use_barrels(std::shared_ptr<BarrelsReader> reader) { barrels_reader = reader; }
    void use_semantic(std::shared_ptr<SemanticEngine> sem) { semantic = sem; }
    
    // Added for Stage 9 compatibility: Attach delta inverted index for query-time merging
    void attach_delta_index(const std::unordered_map<int, std::vector<int>>* delta_inv) {
        delta_inv_index = delta_inv;
    }

    std::vector<SearchResult> search(const std::string& query, int top_k = 5);

private:
    const Lexicon& lexicon;
    const InvertedIndex& inv_index; // Static inverted index
    const ForwardIndex* fwd_index = nullptr;
    const std::unordered_map<int, std::vector<int>>* delta_inv_index = nullptr; // Delta inverted index (Stage 9)
    std::shared_ptr<BarrelsReader> barrels_reader;
    std::shared_ptr<SemanticEngine> semantic; // Stage 7 semantic search
};