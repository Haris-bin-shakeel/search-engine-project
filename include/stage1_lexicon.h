#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <cctype>

class Lexicon {
public:
    // Industry standard: Stopword filtering (single source of truth)
    // Used by: static indexing, dynamic indexing, query parsing, autocomplete
    static bool is_stopword(const std::string& token);
    static std::vector<std::string> tokenize_and_filter(const std::string& text);
public:
    void build_from_docs(const std::vector<std::string>& docs);

    int get_term_id(const std::string& token) const {
        auto it = token_to_id.find(token);
        if (it != token_to_id.end()) return it->second;
        return -1;
    }

    int get_df(int term_id) const {
        auto it = df_map.find(term_id);
        return it != df_map.end() ? it->second : 0;
    }

    const std::unordered_map<std::string,int>& get_token_to_id() const { return token_to_id; }

    // NEW: Get term string from term ID (needed for semantic search)
    std::string get_term_string(int term_id) const {
        auto it = id_to_token.find(term_id);
        return it != id_to_token.end() ? it->second : "";
    }

    // Added for Stage 9 compatibility: Incremental term addition
    int add_or_get_term_id(const std::string& token);
    
    // Added for Stage 9 compatibility: Increment document frequency
    void increment_df(int term_id);

private:
    // Industry standard: Common English stopwords (Lucene/Elasticsearch style)
    static const std::unordered_set<std::string> STOPWORDS;
    
    std::unordered_map<std::string,int> token_to_id;
    std::unordered_map<int,std::string> id_to_token; // reverse mapping
    std::unordered_map<int,int> df_map;
    int next_id = 0;
    
    // Added for Stage 9 compatibility: Allow modification of next_id
    friend class DynamicIndexer;
};