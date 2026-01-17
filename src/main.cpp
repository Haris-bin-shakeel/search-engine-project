#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <chrono>

#include "stage1_lexicon.h"
#include "stage2_forward_index.h"
#include "stage3_inverted_index.h"
#include "stage4_ranking.h"
#include "stage5_query_engine.h"
#include "stage6_barrels.h"
#include "stage7_semantic.h"
#include "stage8_autocomplete.h" // Make sure this is included

int main() {
    // Load your large dataset
    std::string data_path = "./data/corpus_tokens_final_clean.txt";
    std::vector<std::string> documents;
    {
        std::ifstream infile(data_path);
        std::string line;
        while (std::getline(infile, line)) {
            if (!line.empty()) documents.push_back(line);
        }
        std::cout << "Loaded " << documents.size() << " documents from corpus." << std::endl;
    }

    // Stage 1: Lexicon
    Lexicon lex;
    lex.build_from_docs(documents);
    std::cout << "Lexicon built: " << lex.get_token_to_id().size() << " unique tokens." << std::endl;

    // Stage 2: Forward Index
    ForwardIndex fwd_index;
    fwd_index.build_from_docs(documents, lex);
    std::cout << "Forward Index built." << std::endl;

    // Stage 3: Inverted Index
    InvertedIndex inv_index;
    inv_index.build(fwd_index);
    std::cout << "Inverted Index built." << std::endl;

    // Stage 4: Ranking
    Stage4Ranking ranker(fwd_index, lex);
    std::cout << "Ranking Engine ready." << std::endl;

    // Stage 5: Query Engine
    QueryEngine qengine(lex, inv_index); // <-- Use the correct constructor matching Lexicon + InvertedIndex
    qengine.attach_forward_index(fwd_index);
    std::cout << "Query Engine initialized." << std::endl;

    // Stage 6: Barrels (optional)
    std::shared_ptr<BarrelsReader> barrels = std::make_shared<BarrelsReader>();
    qengine.use_barrels(barrels);

    // Stage 7: Semantic Engine
    std::string glove_path = "./data/glove.6B.50d.txt";
    auto semantic = std::make_shared<SemanticEngine>(glove_path, 50);
    semantic->build_document_vectors(documents, lex, ranker);
    qengine.use_semantic(semantic);
    std::cout << "Semantic Engine ready." << std::endl;

    // Stage 8: Autocomplete
    Autocomplete autocomplete(documents, lex); // <-- Use const references in constructor
    autocomplete.build_trie();
    std::cout << "Autocomplete ready." << std::endl;

    // === Test Queries ===
    std::vector<std::string> queries = {"car", "automobile hire", "fast vehicle", "xyzunknown", "services"};
    for (auto& query : queries) {
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "\nQuery: " << query << std::endl;

        auto results = qengine.search(query, 5);
        if (results.empty()) {
            std::cout << "No results found." << std::endl;
        } else {
            for (auto& res : results) {
                std::cout << "DocID: " << res.doc_id 
                          << " | Score: " << res.score 
                          << " | Snippet: " << res.snippet << std::endl;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Query processed in " << ms << " ms." << std::endl;
        std::cout << "--------------------------" << std::endl;
    }

    // === Autocomplete Test ===
    std::vector<std::string> prefixes = {"car", "auto", "fast", "serv", "xyz"};
    std::cout << "\n=== Autocomplete Test ===" << std::endl;
    for (auto& prefix : prefixes) {
        auto suggestions = autocomplete.get_suggestions(prefix, 5);
        std::cout << "Prefix: \"" << prefix << "\" | Suggestions: ";
        for (auto& s : suggestions) std::cout << s << " ";
        std::cout << std::endl;
    }

    std::cout << "\n=== Pipeline Test Completed ===" << std::endl;
    return 0;
}
