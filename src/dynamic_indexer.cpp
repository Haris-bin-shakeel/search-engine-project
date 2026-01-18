#include "dynamic_indexer.h"
#include <iostream>
#include <cctype>
#include <sstream>
#include <unordered_set>
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
// Fallback for older compilers - use basic file operations
#include <sys/stat.h>
#include <sys/types.h>
namespace fs {
    inline bool exists(const std::string& path) {
        std::ifstream f(path);
        return f.good();
    }
    inline void create_directories(const std::string& path) {
        // Simple fallback - just try to create the directory
        // In production, use platform-specific code
    }
}
#endif

DynamicIndexer::DynamicIndexer(Lexicon& lex, ForwardIndex& fwd, InvertedIndex& inv, Stage4Ranking& rank)
    : lexicon(lex), forward_index(fwd), inverted_index(inv), ranking(rank)
{
    // Initialize next_doc_id based on existing forward index size
    static_doc_count = static_cast<int>(forward_index.getIndex().size());
    next_doc_id = static_doc_count;
}

void DynamicIndexer::add_document(const std::string& document_text) {
    if (document_text.empty()) {
        std::cerr << "[Stage 9] Warning: Attempted to add empty document\n";
        return;
    }
    
    // Tokenize document
    std::vector<std::string> tokens = tokenize(document_text);
    if (tokens.empty()) {
        std::cerr << "[Stage 9] Warning: No tokens found in document\n";
        return;
    }
    
    // Assign new document ID
    int doc_id = next_doc_id++;
    
    // Process tokens: add to lexicon and collect term IDs
    std::vector<int> term_ids;
    std::unordered_set<int> unique_terms; // Track unique terms for DF increment
    std::unordered_set<int> new_term_ids; // Track newly added terms
    
    for (const auto& token : tokens) {
        // Check if token already exists
        int existing_term_id = lexicon.get_term_id(token);
        int term_id;
        
        if (existing_term_id == -1) {
            // New term
            term_id = lexicon.add_or_get_term_id(token);
            new_term_ids.insert(term_id);
        } else {
            term_id = existing_term_id;
        }
        
        term_ids.push_back(term_id);
        
        // Track unique terms for this document
        unique_terms.insert(term_id);
    }
    
    // Increment DF for each unique term in this document
    for (int term_id : unique_terms) {
        lexicon.increment_df(term_id);
    }
    
    // Add to forward index
    forward_index.add_document(doc_id, term_ids);
    
    // Industry standard: Add to DELTA inverted index (NOT static index)
    // Static index remains unchanged - delta merged at query time
    for (int term_id : term_ids) {
        delta_inv_index[term_id].push_back(doc_id);
    }
    
    // Update ranking stats
    ranking.update_stats();
    
    // Persist immediately to disk (only this document's data)
    std::string delta_dir = "./data";
    persist_to_disk(doc_id, term_ids, new_term_ids);
    
    std::cout << "[Stage 9] Document " << doc_id << " indexed and persisted (" 
              << term_ids.size() << " terms, " << unique_terms.size() << " unique)\n";
}

std::vector<std::string> DynamicIndexer::tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::istringstream iss(text);
    std::string token;
    
    while (iss >> token) {
        // Convert to lowercase
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

// Private overload: persist single document's data
void DynamicIndexer::persist_to_disk(int doc_id, const std::vector<int>& term_ids, const std::unordered_set<int>& new_term_ids) {
    std::string delta_dir = "./data";
    
    // Create delta directory if it doesn't exist
    fs::create_directories(delta_dir);
    
    // Append to delta files (append-only strategy)
    std::string forward_file = delta_dir + "/delta_forward_index.dat";
    std::string inverted_file = delta_dir + "/delta_inverted_index.dat";
    std::string lexicon_file = delta_dir + "/delta_lexicon.dat";
    std::string stats_file = delta_dir + "/delta_stats.dat";
    
    // Save forward index delta (this document only)
    save_forward_delta(forward_file, doc_id, term_ids);
    
    // Save inverted index delta (postings for this document's terms)
    // For each unique term in this document, save posting (term_id -> doc_id)
    std::unordered_set<int> unique_terms(term_ids.begin(), term_ids.end());
    for (int term_id : unique_terms) {
        save_inverted_delta(inverted_file, term_id, std::vector<int>{doc_id});
    }
    
    // Save lexicon delta (only new terms)
    for (int term_id : new_term_ids) {
        std::string token = lexicon.get_term_string(term_id);
        if (!token.empty()) {
            save_lexicon_delta(lexicon_file, token, term_id);
        }
    }
    
    // Save stats
    save_stats(stats_file);
}

// Public overload: save current state (for manual calls)
void DynamicIndexer::persist_to_disk(const std::string& delta_dir) {
    // Create delta directory if it doesn't exist
    fs::create_directories(delta_dir);
    
    // Just save stats (forward/inverted/lexicon are saved incrementally)
    std::string stats_file = delta_dir + "/delta_stats.dat";
    save_stats(stats_file);
}

void DynamicIndexer::save_forward_delta(const std::string& filepath, int doc_id, const std::vector<int>& term_ids) {
    std::ofstream out(filepath, std::ios::app | std::ios::binary);
    if (!out.is_open()) return;
    
    // Format: doc_id (int) | num_terms (int) | term_ids... (int[])
    out.write(reinterpret_cast<const char*>(&doc_id), sizeof(int));
    int num_terms = static_cast<int>(term_ids.size());
    out.write(reinterpret_cast<const char*>(&num_terms), sizeof(int));
    out.write(reinterpret_cast<const char*>(term_ids.data()), num_terms * sizeof(int));
    out.close();
}

void DynamicIndexer::save_inverted_delta(const std::string& filepath, int term_id, const std::vector<int>& doc_ids) {
    std::ofstream out(filepath, std::ios::app | std::ios::binary);
    if (!out.is_open()) return;
    
    // Format: term_id (int) | num_docs (int) | doc_ids... (int[])
    out.write(reinterpret_cast<const char*>(&term_id), sizeof(int));
    int num_docs = static_cast<int>(doc_ids.size());
    out.write(reinterpret_cast<const char*>(&num_docs), sizeof(int));
    out.write(reinterpret_cast<const char*>(doc_ids.data()), num_docs * sizeof(int));
    out.close();
}

void DynamicIndexer::save_lexicon_delta(const std::string& filepath, const std::string& token, int term_id) {
    std::ofstream out(filepath, std::ios::app);
    if (!out.is_open()) return;
    
    // Format: term_id | token\n
    out << term_id << " " << token << "\n";
    out.close();
}

void DynamicIndexer::save_stats(const std::string& filepath) {
    std::ofstream out(filepath);
    if (!out.is_open()) return;
    
    // Format: next_doc_id | static_doc_count | avg_doc_len
    out << next_doc_id << " " << static_doc_count << " " << ranking.get_avg_doc_len() << "\n";
    out.close();
}

int DynamicIndexer::load_delta_index(const std::string& delta_dir) {
    int loaded_count = 0;
    
    std::string forward_file = delta_dir + "/delta_forward_index.dat";
    std::string inverted_file = delta_dir + "/delta_inverted_index.dat";
    std::string lexicon_file = delta_dir + "/delta_lexicon.dat";
    std::string stats_file = delta_dir + "/delta_stats.dat";
    
    // Load stats first to get next_doc_id
    if (fs::exists(stats_file)) {
        load_stats(stats_file);
        std::cout << "[Stage 9] Loaded stats: next_doc_id=" << next_doc_id 
                  << ", static_doc_count=" << static_doc_count << "\n";
    }
    
    // Load lexicon delta
    if (fs::exists(lexicon_file)) {
        load_lexicon_delta(lexicon_file);
        std::cout << "[Stage 9] Loaded lexicon delta\n";
    }
    
    // Load forward index delta
    if (fs::exists(forward_file)) {
        load_forward_delta(forward_file);
        loaded_count = static_cast<int>(forward_index.getIndex().size()) - static_doc_count;
        std::cout << "[Stage 9] Loaded " << loaded_count << " documents from forward delta\n";
    }
    
    // Load inverted index delta
    if (fs::exists(inverted_file)) {
        load_inverted_delta(inverted_file);
        std::cout << "[Stage 9] Loaded inverted index delta\n";
    }
    
    // Update ranking stats after loading
    if (loaded_count > 0) {
        ranking.update_stats();
        std::cout << "[Stage 9] Updated ranking statistics\n";
    }
    
    return loaded_count;
}

void DynamicIndexer::load_forward_delta(const std::string& filepath) {
    std::ifstream in(filepath, std::ios::binary);
    if (!in.is_open()) return;
    
    while (in.good()) {
        int doc_id;
        int num_terms;
        
        in.read(reinterpret_cast<char*>(&doc_id), sizeof(int));
        if (!in.good()) break;
        
        in.read(reinterpret_cast<char*>(&num_terms), sizeof(int));
        if (!in.good()) break;
        
        std::vector<int> term_ids(num_terms);
        in.read(reinterpret_cast<char*>(term_ids.data()), num_terms * sizeof(int));
        if (!in.good()) break;
        
        // Add to forward index
        forward_index.add_document(doc_id, term_ids);
    }
    
    in.close();
}

void DynamicIndexer::load_inverted_delta(const std::string& filepath) {
    std::ifstream in(filepath, std::ios::binary);
    if (!in.is_open()) return;
    
    while (in.good()) {
        int term_id;
        int num_docs;
        
        in.read(reinterpret_cast<char*>(&term_id), sizeof(int));
        if (!in.good()) break;
        
        in.read(reinterpret_cast<char*>(&num_docs), sizeof(int));
        if (!in.good()) break;
        
        std::vector<int> doc_ids(num_docs);
        in.read(reinterpret_cast<char*>(doc_ids.data()), num_docs * sizeof(int));
        if (!in.good()) break;
        
        // Industry standard: Load into DELTA inverted index (not static)
        for (int doc_id : doc_ids) {
            delta_inv_index[term_id].push_back(doc_id);
        }
    }
    
    in.close();
}

void DynamicIndexer::load_lexicon_delta(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in.is_open()) return;
    
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        
        std::istringstream iss(line);
        int term_id;
        std::string token;
        
        if (iss >> term_id >> token) {
            // Add to lexicon if not exists
            std::string lower_token = token;
            std::transform(lower_token.begin(), lower_token.end(), lower_token.begin(), ::tolower);
            
            // Check if already exists
            if (lexicon.get_term_id(lower_token) == -1) {
                // Add using friend access to next_id
                // Since we're loading, we need to ensure next_id is set correctly
                lexicon.add_or_get_term_id(lower_token);
            }
        }
    }
    
    in.close();
}

void DynamicIndexer::load_stats(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in.is_open()) return;
    
    in >> next_doc_id >> static_doc_count;
    double avg_doc_len;
    in >> avg_doc_len;
    
    in.close();
}

// Industry standard: Delta compaction (offline job)
int DynamicIndexer::compact_delta_to_static() {
    int delta_doc_count = static_cast<int>(forward_index.getIndex().size()) - static_doc_count;
    
    if (delta_doc_count <= 0) {
        std::cout << "[COMPACT] No delta documents to compact.\n";
        return 0;
    }
    
    std::cout << "[COMPACT] Starting compaction: merging " << delta_doc_count 
              << " delta documents into static index...\n";
    
    // Step 1: Merge delta inverted index into static inverted index
    std::cout << "[COMPACT] Merging inverted index...\n";
    for (const auto& [term_id, doc_ids] : delta_inv_index) {
        for (int doc_id : doc_ids) {
            // Merge each posting into static index
            inverted_index.add_document(doc_id, std::vector<int>{term_id});
        }
    }
    
    // Step 2: Clear delta inverted index (in-memory)
    delta_inv_index.clear();
    
    // Step 3: Update static document count
    static_doc_count = static_cast<int>(forward_index.getIndex().size());
    next_doc_id = static_doc_count; // Reset to current size
    
    // Step 4: Update ranking stats
    std::cout << "[COMPACT] Updating ranking statistics...\n";
    ranking.update_stats();
    
    // Step 5: Clear delta files on disk
    std::string delta_dir = "./data";
    std::string forward_file = delta_dir + "/delta_forward_index.dat";
    std::string inverted_file = delta_dir + "/delta_inverted_index.dat";
    std::string lexicon_file = delta_dir + "/delta_lexicon.dat";
    std::string stats_file = delta_dir + "/delta_stats.dat";
    
    std::cout << "[COMPACT] Clearing delta files...\n";
    std::remove(forward_file.c_str());
    std::remove(inverted_file.c_str());
    std::remove(lexicon_file.c_str());
    std::remove(stats_file.c_str());
    
    // Step 6: Save updated stats (with cleared delta)
    save_stats(stats_file);
    
    std::cout << "[COMPACT] Compaction complete! " << delta_doc_count 
              << " documents merged into static index.\n";
    std::cout << "[COMPACT] Delta index cleared. Next startup will load 0 delta documents.\n";
    
    return delta_doc_count;
}
