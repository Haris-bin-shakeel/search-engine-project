#include "stage1_lexicon.h"
#include <sstream>
#include <algorithm>

// Industry standard: Common English stopwords (Lucene/Elasticsearch default set)
const std::unordered_set<std::string> Lexicon::STOPWORDS = {
    "a", "an", "and", "are", "as", "at", "be", "by", "for", "from",
    "has", "he", "in", "is", "it", "its", "of", "on", "that", "the",
    "to", "was", "will", "with", "the", "this", "but", "they", "have",
    "had", "what", "said", "each", "which", "their", "time", "if",
    "up", "out", "many", "then", "them", "these", "so", "some", "her",
    "would", "make", "like", "into", "him", "has", "two", "more",
    "very", "after", "words", "long", "than", "first", "been", "call",
    "who", "oil", "sit", "now", "find", "down", "day", "did", "get",
    "come", "made", "may", "part"
};

// Industry standard: Check if token is a stopword
bool Lexicon::is_stopword(const std::string& token) {
    std::string lower_token = token;
    std::transform(lower_token.begin(), lower_token.end(), lower_token.begin(), ::tolower);
    return STOPWORDS.find(lower_token) != STOPWORDS.end();
}

// Industry standard: Tokenize text and filter stopwords (single source of truth)
std::vector<std::string> Lexicon::tokenize_and_filter(const std::string& text) {
    std::vector<std::string> tokens;
    std::istringstream iss(text);
    std::string token;
    
    while (iss >> token) {
        // Convert to lowercase
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        
        // Filter stopwords
        if (!token.empty() && !is_stopword(token)) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

void Lexicon::build_from_docs(const std::vector<std::string>& docs) {
    for (const auto& doc : docs) {
        // Industry standard: Use centralized tokenization with stopword filtering
        std::vector<std::string> tokens = tokenize_and_filter(doc);
        
        for (const auto& token : tokens) {
            // Add token if not exists
            if (token_to_id.find(token) == token_to_id.end()) {
                token_to_id[token] = next_id;
                id_to_token[next_id] = token; // reverse mapping
                ++next_id;
            }

            // Increase DF count
            int term_id = token_to_id[token];
            df_map[term_id]++;
        }
    }
}

// Added for Stage 9 compatibility: Incremental term addition
int Lexicon::add_or_get_term_id(const std::string& token) {
    std::string lower_token = token;
    std::transform(lower_token.begin(), lower_token.end(), lower_token.begin(), ::tolower);
    
    // Industry standard: Stopwords should not be added to lexicon
    if (is_stopword(lower_token)) {
        return -1; // Indicate stopword (caller should skip)
    }
    
    auto it = token_to_id.find(lower_token);
    if (it != token_to_id.end()) {
        return it->second;
    }
    
    // New term
    int term_id = next_id++;
    token_to_id[lower_token] = term_id;
    id_to_token[term_id] = lower_token;
    df_map[term_id] = 0; // Will be incremented by caller
    return term_id;
}

// Added for Stage 9 compatibility: Increment document frequency
void Lexicon::increment_df(int term_id) {
    df_map[term_id]++;
}