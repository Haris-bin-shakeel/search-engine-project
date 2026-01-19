#include "stage7_semantic.h"
#include "stage1_lexicon.h"
#include "stage4_ranking.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

SemanticEngine::SemanticEngine(const std::string& glove_file, int dim)
    : dimension(dim) // initialize dimension
{
    std::ifstream infile(glove_file);
    if (!infile) throw std::runtime_error("Cannot open GloVe file: " + glove_file);

    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string word;
        iss >> word;
        std::vector<double> vec(dim, 0.0);
        for (int i = 0; i < dim; ++i) iss >> vec[i];
        embeddings[word] = vec;
    }
}

void SemanticEngine::build_document_vectors(const std::vector<std::string>& documents,
                                            const Lexicon& lex,
                                            const Stage4Ranking& ranker)
{
    doc_vectors.clear();
    for (const auto& doc : documents) {
        std::vector<double> vec(dimension, 0.0);
        std::istringstream iss(doc);
        std::string token;
        int count = 0;
        while (iss >> token) {
            std::transform(token.begin(), token.end(), token.begin(), ::tolower);
            if (embeddings.find(token) != embeddings.end()) {
                for (int i = 0; i < dimension; ++i) vec[i] += embeddings[token][i];
                ++count;
            }
        }
        if (count > 0)
            for (int i = 0; i < dimension; ++i) vec[i] /= count;
        doc_vectors.push_back(vec);
    }
}

void SemanticEngine::rerank(const std::string& query,
                            std::vector<SearchResult>& results,
                            const Lexicon& lex,
                            const Stage4Ranking& ranker)
{
    std::vector<double> query_vec(dimension, 0.0);
    std::istringstream iss(query);
    std::string token;
    int count = 0;
    while (iss >> token) {
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        if (embeddings.find(token) != embeddings.end()) {
            for (int i = 0; i < dimension; ++i) query_vec[i] += embeddings[token][i];
            ++count;
        }
    }
    if (count > 0)
        for (int i = 0; i < dimension; ++i) query_vec[i] /= count;

    for (auto& res : results) {
        if (res.doc_id >= doc_vectors.size()) continue;
        const auto& doc_vec = doc_vectors[res.doc_id];

        double dot = 0.0, norm_q = 0.0, norm_d = 0.0;
        for (int i = 0; i < dimension; ++i) {
            dot += query_vec[i] * doc_vec[i];
            norm_q += query_vec[i] * query_vec[i];
            norm_d += doc_vec[i] * doc_vec[i];
        }
        double cos_sim = (norm_q && norm_d) ? dot / (std::sqrt(norm_q) * std::sqrt(norm_d)) : 0.0;

        res.score = 0.7 * res.score + 0.3 * cos_sim;
    }
}

// Semantic search for debug mode (returns cosine similarity scores only)
std::vector<SearchResult> SemanticEngine::semantic_search(const std::string& query, int top_k) {
    std::vector<SearchResult> results;
    
    // Build query vector (same logic as rerank)
    std::vector<double> query_vec(dimension, 0.0);
    std::istringstream iss(query);
    std::string token;
    int count = 0;
    while (iss >> token) {
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        if (embeddings.find(token) != embeddings.end()) {
            for (int i = 0; i < dimension; ++i) query_vec[i] += embeddings[token][i];
            ++count;
        }
    }
    if (count > 0)
        for (int i = 0; i < dimension; ++i) query_vec[i] /= count;
    
    // Compute cosine similarity for all documents
    for (size_t doc_id = 0; doc_id < doc_vectors.size(); ++doc_id) {
        const auto& doc_vec = doc_vectors[doc_id];
        
        double dot = 0.0, norm_q = 0.0, norm_d = 0.0;
        for (int i = 0; i < dimension; ++i) {
            dot += query_vec[i] * doc_vec[i];
            norm_q += query_vec[i] * query_vec[i];
            norm_d += doc_vec[i] * doc_vec[i];
        }
        double cos_sim = (norm_q && norm_d) ? dot / (std::sqrt(norm_q) * std::sqrt(norm_d)) : 0.0;
        
        if (cos_sim > 0.0) { // Only include documents with some similarity
            SearchResult res;
            res.doc_id = static_cast<int>(doc_id);
            res.score = cos_sim; // Pure cosine similarity (no BM25 mixing)
            res.snippet = "";
            results.push_back(res);
        }
    }
    
    // Sort by cosine similarity descending
    std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        return a.score > b.score;
    });
    
    if (results.size() > (size_t)top_k) results.resize(top_k);
    return results;
}