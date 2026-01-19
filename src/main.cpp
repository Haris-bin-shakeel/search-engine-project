#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

#include "stage1_lexicon.h"
#include "stage2_forward_index.h"
#include "stage3_inverted_index.h"
#include "stage4_ranking.h"
#include "stage5_query_engine.h"
#include "stage6_barrels.h"
#include "stage7_semantic.h"
#include "stage8_autocomplete.h"
#include "dynamic_indexer.h"

// Helper: Trim whitespace from string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Helper: Parse ADD command (format: ADD: document text)
bool parse_add_command(const std::string& input, std::string& document_text) {
    if (input.size() < 5) return false;
    
    std::string prefix = input.substr(0, 4);
    std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::toupper);
    
    if (prefix == "ADD:") {
        document_text = trim(input.substr(4));
        return !document_text.empty();
    }
    
    return false;
}

// Semantic debug helper (NO side effects - read-only demonstration)
void semantic_debug(
    const std::string& query,
    std::shared_ptr<SemanticEngine> semantic,
    int top_k = 5
) {
    std::cout << "\n[SEMANTIC DEBUG MODE]\n";
    std::cout << "Query: \"" << query << "\"\n";

    auto results = semantic->semantic_search(query, top_k);

    if (results.empty()) {
        std::cout << "No semantic matches found.\n";
        return;
    }

    std::cout << "Top semantic matches (cosine similarity):\n";
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "  " << (i + 1)
                  << ". DocID: " << results[i].doc_id
                  << " | Similarity: " << results[i].score
                  << std::endl;
    }

    std::cout << "[END SEMANTIC DEBUG]\n\n";
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   Search Engine - Industry Grade CLI" << std::endl;
    std::cout << "   Stages 1-9 Implementation" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // ============================================================
    // INITIALIZATION PHASE
    // ============================================================
    std::cout << "[INIT] Starting initialization phase...\n" << std::endl;
    
    // Load corpus from file
    std::string data_path = "./data/corpus_tokens_final_clean.txt";
    std::vector<std::string> documents;
    {
        std::ifstream infile(data_path);
        if (!infile.is_open()) {
            std::cerr << "[ERROR] Cannot open corpus file: " << data_path << std::endl;
            return 1;
        }
        std::string line;
        while (std::getline(infile, line)) {
            if (!line.empty()) documents.push_back(line);
        }
        std::cout << "[Stage 1-8] Loaded " << documents.size() << " documents from corpus." << std::endl;
    }
    
    // Stage 1: Lexicon
    std::cout << "[Stage 1] Building Lexicon..." << std::endl;
    Lexicon lex;
    lex.build_from_docs(documents);
    std::cout << "[Stage 1] Lexicon size: " << lex.get_token_to_id().size() << " unique tokens." << std::endl;
    
    // Stage 2: Forward Index
    std::cout << "[Stage 2] Building Forward Index..." << std::endl;
    ForwardIndex fwd_index;
    fwd_index.build_from_docs(documents, lex);
    std::cout << "[Stage 2] Total documents: " << fwd_index.getIndex().size() << std::endl;
    
    // Stage 3: Inverted Index
    std::cout << "[Stage 3] Building Inverted Index..." << std::endl;
    InvertedIndex inv_index;
    inv_index.build(fwd_index);
    std::cout << "[Stage 3] Inverted terms: " << inv_index.getIndex().size() << std::endl;
    
    // Stage 4: Ranking
    std::cout << "[Stage 4] Computing Ranking Statistics..." << std::endl;
    Stage4Ranking ranker(fwd_index, lex);
    std::cout << "[Stage 4] Avg document length updated: " << ranker.get_avg_doc_len() << std::endl;
    
    // Stage 5: Query Engine
    std::cout << "[Stage 5] Initializing Query Engine..." << std::endl;
    QueryEngine qengine(lex, inv_index);
    qengine.attach_forward_index(fwd_index);
    std::cout << "[Stage 5] Query Engine initialized." << std::endl;
    
    // Stage 6: Barrels
    std::cout << "[Stage 6] Initializing Barrels Reader..." << std::endl;
    std::shared_ptr<BarrelsReader> barrels = std::make_shared<BarrelsReader>();
    qengine.use_barrels(barrels);
    std::cout << "[Stage 6] Barrels ready." << std::endl;
    
    // Stage 7: Semantic Engine
    std::cout << "[Stage 7] Loading Semantic Engine..." << std::endl;
    std::string glove_path = "./data/glove.6B.50d.txt";
    auto semantic = std::make_shared<SemanticEngine>(glove_path, 50);
    semantic->build_document_vectors(documents, lex, ranker);
    qengine.use_semantic(semantic);
    std::cout << "[Stage 7] Semantic Engine ready." << std::endl;
    
    // Stage 8: Autocomplete
    std::cout << "[Stage 8] Building Autocomplete Trie..." << std::endl;
    Autocomplete autocomplete(documents, lex);
    autocomplete.build_trie();
    std::cout << "[Stage 8] Autocomplete ready." << std::endl;
    
    // Stage 9: Dynamic Indexer
    std::cout << "[Stage 9] Initializing Dynamic Indexer..." << std::endl;
    DynamicIndexer dynamic_indexer(lex, fwd_index, inv_index, ranker);
    
    // Load delta index if present
    int delta_docs = dynamic_indexer.load_delta_index("./data");
    if (delta_docs > 0) {
        std::cout << "[Stage 9] Loaded " << delta_docs << " documents from delta index." << std::endl;
    } else {
        std::cout << "[Stage 9] No delta index found (first run)." << std::endl;
    }
    
    // Industry standard: Attach delta index to QueryEngine for query-time merging
    qengine.attach_delta_index(&dynamic_indexer.get_delta_inverted_index());
    std::cout << "[Stage 9] Delta index attached to QueryEngine (query-time merge enabled)." << std::endl;
    
    // Rebuild autocomplete from lexicon (to include any loaded delta terms)
    autocomplete.rebuild_from_lexicon();
    std::cout << "[Stage 8] Autocomplete rebuilt from lexicon (includes delta terms)." << std::endl;
    
    std::cout << "\n[INIT] Initialization complete!\n" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // ============================================================
    // RUNTIME INTERACTIVE TESTING
    // ============================================================
    std::cout << "Interactive CLI Ready. Commands:" << std::endl;
    std::cout << "  - Enter query text to search" << std::endl;
    std::cout << "  - ADD: <text> to add new document" << std::endl;
    std::cout << "  - AUTO: <prefix> for autocomplete" << std::endl;
    std::cout << "  - SEMANTIC: <query> for semantic-only search (debug)" << std::endl;
    std::cout << "  - COMPACT to merge delta into static index" << std::endl;
    std::cout << "  - EXIT or QUIT to exit\n" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (!std::cin.good()) {
            std::cout << "\n[INFO] Exiting..." << std::endl;
            break;
        }
        
        input = trim(input);
        if (input.empty()) continue;
        
        // Convert to uppercase for command checking
        std::string upper_input = input;
        std::transform(upper_input.begin(), upper_input.end(), upper_input.begin(), ::toupper);
        
        // Handle EXIT/QUIT
        if (upper_input == "EXIT" || upper_input == "QUIT") {
            std::cout << "[INFO] Exiting..." << std::endl;
            break;
        }
        
        // Handle ADD command
        std::string doc_text;
        if (parse_add_command(input, doc_text)) {
            std::cout << "[Stage 9] Adding document dynamically..." << std::endl;
            auto start = std::chrono::high_resolution_clock::now();
            
            dynamic_indexer.add_document(doc_text);
            
            // Industry standard: Update autocomplete after adding new terms
            autocomplete.rebuild_from_lexicon();
            
            auto end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::cout << "[Stage 9] Document persisted to disk in " << ms << " ms." << std::endl;
            std::cout << "[Stage 9] Autocomplete updated with new terms.\n" << std::endl;
            continue;
        }
        
        // Handle COMPACT command (Industry standard: Delta compaction)
        if (upper_input == "COMPACT") {
            std::cout << "[COMPACT] Starting delta compaction (offline job)..." << std::endl;
            auto start = std::chrono::high_resolution_clock::now();
            
            int compacted = dynamic_indexer.compact_delta_to_static();
            
            // Rebuild inverted index from merged forward index (ensure consistency)
            // Note: Static index already merged, but we need to rebuild for consistency
            inv_index.build(fwd_index);
            
            // Reattach delta index (now empty) to QueryEngine
            qengine.attach_delta_index(&dynamic_indexer.get_delta_inverted_index());
            
            auto end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::cout << "[COMPACT] Compaction completed in " << ms << " ms." << std::endl;
            std::cout << "[COMPACT] " << compacted << " documents merged into static index.\n" << std::endl;
            continue;
        }
        
        // Handle SEMANTIC debug command (Industry standard: semantic search demonstration)
        if (upper_input.size() > 9 && upper_input.substr(0, 9) == "SEMANTIC:") {
            std::string query = trim(input.substr(9));
            if (!query.empty()) {
                semantic_debug(query, semantic, 5);
            }
            continue;
        }
        
        // Handle AUTocomplete command
        if (upper_input.size() > 5 && upper_input.substr(0, 5) == "AUTO:") {
            std::string prefix = trim(input.substr(5));
            if (!prefix.empty()) {
                std::cout << "[Stage 8] Autocomplete suggestions for \"" << prefix << "\":" << std::endl;
                auto suggestions = autocomplete.get_suggestions(prefix, 5);
                if (suggestions.empty()) {
                    std::cout << "  No suggestions found." << std::endl;
                } else {
                    for (size_t i = 0; i < suggestions.size(); ++i) {
                        std::cout << "  " << (i + 1) << ". " << suggestions[i] << std::endl;
                    }
                }
                std::cout << std::endl;
            }
            continue;
        }
        
        // Handle normal search query
        std::cout << "[Stage 5] Processing query: \"" << input << "\"" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        
        auto results = qengine.search(input, 5);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        if (results.empty()) {
            std::cout << "  No results found." << std::endl;
        } else {
            std::cout << "  Found " << results.size() << " results:" << std::endl;
            for (size_t i = 0; i < results.size(); ++i) {
                const auto& res = results[i];
                std::cout << "  " << (i + 1) << ". DocID: " << res.doc_id 
                          << " | Score: " << res.score;
                if (!res.snippet.empty()) {
                    std::cout << " | Snippet: " << res.snippet;
                }
                std::cout << std::endl;
            }
        }
        
        std::cout << "[Stage 5] Query processed in " << ms << " ms.\n" << std::endl;
    }
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "   Search Engine Session Ended" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
