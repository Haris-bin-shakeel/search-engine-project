#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include "stage1_lexicon.h"
#include "stage2_forward_index.h"
#include "stage3_inverted_index.h"
#include "stage4_ranking.h"

/**
 * Stage 9: Dynamic Indexer with Disk Persistence
 * 
 * Provides incremental document indexing at runtime with append-only
 * delta indexing strategy. New documents are persisted immediately to
 * disk as deltas, separate from the static corpus index.
 */
class DynamicIndexer {
public:
    DynamicIndexer(Lexicon& lex, ForwardIndex& fwd, InvertedIndex& inv, Stage4Ranking& rank);
    
    /**
     * Add a new document to the index incrementally
     * Updates lexicon, forward index, inverted index, and ranking stats
     * Persists changes to disk immediately
     */
    void add_document(const std::string& document_text);
    
    /**
     * Load delta index from disk on startup
     * Returns number of documents loaded
     */
    int load_delta_index(const std::string& delta_dir = "./data");
    
    /**
     * Persist current state to disk (called after each document addition)
     */
    void persist_to_disk(const std::string& delta_dir = "./data");
    
    /**
     * Get the next document ID that will be assigned
     */
    int get_next_doc_id() const { return next_doc_id; }
    
    /**
     * Set the next document ID (used when loading delta index)
     */
    void set_next_doc_id(int doc_id) { next_doc_id = doc_id; }
    
    /**
     * Get delta inverted index for query-time merging
     * Industry standard: separate static + delta indexes
     */
    const std::unordered_map<int, std::vector<int>>& get_delta_inverted_index() const {
        return delta_inv_index;
    }
    
    /**
     * Industry standard: Delta compaction (offline job)
     * Merges delta index into static index, clears delta, updates ranking stats
     * Returns number of documents compacted
     */
    int compact_delta_to_static();

private:
    Lexicon& lexicon;
    ForwardIndex& forward_index;
    InvertedIndex& inverted_index; // Static index (read-only for new docs)
    Stage4Ranking& ranking;
    
    // Industry standard: Separate delta inverted index (LSM-style)
    std::unordered_map<int, std::vector<int>> delta_inv_index;
    
    int next_doc_id = 0; // Tracks next document ID to assign
    int static_doc_count = 0; // Original corpus size (for ID offset)
    
    // Helper: Tokenize document text
    std::vector<std::string> tokenize(const std::string& text);
    
    // Disk persistence helper (private overload for single document)
    void persist_to_disk(int doc_id, const std::vector<int>& term_ids, const std::unordered_set<int>& new_term_ids);
    
    // Disk persistence helpers
    void save_forward_delta(const std::string& filepath, int doc_id, const std::vector<int>& term_ids);
    void save_inverted_delta(const std::string& filepath, int term_id, const std::vector<int>& doc_ids);
    void save_lexicon_delta(const std::string& filepath, const std::string& token, int term_id);
    void save_stats(const std::string& filepath);
    
    // Disk loading helpers
    void load_forward_delta(const std::string& filepath);
    void load_inverted_delta(const std::string& filepath);
    void load_lexicon_delta(const std::string& filepath);
    void load_stats(const std::string& filepath);
};
