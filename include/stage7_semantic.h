#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "stage4_ranking.h"
#include "stage5_query_engine.h" // For SearchResult

class SemanticEngine {
public:
    SemanticEngine(const std::string& glove_file, int dim);

    void build_document_vectors(const std::vector<std::string>& documents,
                                const Lexicon& lex,
                                const Stage4Ranking& ranker);

    void rerank(const std::string& query,
                std::vector<SearchResult>& results,
                const Lexicon& lex,
                const Stage4Ranking& ranker);
    
    // Semantic search for debug mode (returns cosine similarity scores)
    std::vector<SearchResult> semantic_search(const std::string& query, int top_k = 5);

private:
    int dimension; // <-- declare dimension here
    std::unordered_map<std::string, std::vector<double>> embeddings;
    std::vector<std::vector<double>> doc_vectors;
};